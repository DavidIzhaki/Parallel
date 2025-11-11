// tree_thr_pool_demo.c — parallel in-order using ONLY thr_pool
// Build: cc -O3 -pthread tree_thr_pool_demo.c thr_pool.c -o tree_thr_pool_demo
// Run:   ./tree_thr_pool_demo <N> [grain] [threads]

#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include "thr_pool.h"

static inline double now_sec(void){
    struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

/* ===== tree ===== */
typedef struct Node {
    int data;
    struct Node *left, *right;
    int leftNodeNum, rightNodeNum; /* subtree sizes */
} Node;

static Node* new_node(int x){ Node *n = (Node*)calloc(1,sizeof(Node)); n->data = x; return n; }

static Node* build_balanced_range(int lo, int hi){
    if (lo > hi) return NULL;
    int mid = lo + (hi - lo)/2;
    Node *u = new_node(mid);
    u->left  = build_balanced_range(lo, mid-1);
    u->right = build_balanced_range(mid+1, hi);
    return u;
}

static Node* build_bst_sorted(int N){ return build_balanced_range(1, N); }

static inline int subtree_size(const Node *u){ return 1 + u->leftNodeNum + u->rightNodeNum; }

/* ===== pass #1: serial post-order sizes ===== */
static int fill_sizes_serial(Node *u){
    if (!u) return 0;
    int L = fill_sizes_serial(u->left);
    int R = fill_sizes_serial(u->right);
    u->leftNodeNum  = L;
    u->rightNodeNum = R;
    return 1 + L + R;
}

/* ===== serial in-order fill (used inside coarse tasks) ===== */
static void fill_inorder_serial(Node *u, int base, int *out){
    if (!u) return;
    fill_inorder_serial(u->left, base, out);
    out[base + u->leftNodeNum] = u->data;
    fill_inorder_serial(u->right, base + u->leftNodeNum + 1, out);
}

/* ===== parallel top-down fill via thread pool (coarse tasks) ===== */
typedef struct {
    Node *u;
    int base;
    int *out;
    int grain;
} FillTask;

typedef struct {
    FillTask *buf;
    int cap;
    _Atomic int next;
} TaskArena;

static FillTask* arena_alloc(TaskArena *A){
    int i = atomic_fetch_add(&A->next, 1);
    if (i >= A->cap) return NULL;
    return &A->buf[i];
}

static thr_pool_t *g_pool = NULL;
static TaskArena   g_arena;

static void *fill_task_run(void *arg){
    FillTask t = *(FillTask*)arg; /* copy by value */
    Node *u = t.u;
    if (!u) return NULL;

    int size = subtree_size(u);
    if (size <= t.grain){
        fill_inorder_serial(u, t.base, t.out);
        return NULL;
    }

    /* visit current */
    t.out[t.base + u->leftNodeNum] = u->data;

    /* left */
    if (u->left){
        int Lsz = subtree_size(u->left);
        if (Lsz > t.grain){
            FillTask *L = arena_alloc(&g_arena);
            if (L){
                L->u = u->left; L->base = t.base; L->out = t.out; L->grain = t.grain;
                (void)thr_pool_queue(g_pool, fill_task_run, L);
            } else {
                fill_inorder_serial(u->left, t.base, t.out);
            }
        } else {
            fill_inorder_serial(u->left, t.base, t.out);
        }
    }

    /* right */
    if (u->right){
        int Rbase = t.base + u->leftNodeNum + 1;
        int Rsz = subtree_size(u->right);
        if (Rsz > t.grain){
            FillTask *R = arena_alloc(&g_arena);
            if (R){
                R->u = u->right; R->base = Rbase; R->out = t.out; R->grain = t.grain;
                (void)thr_pool_queue(g_pool, fill_task_run, R);
            } else {
                fill_inorder_serial(u->right, Rbase, t.out);
            }
        } else {
            fill_inorder_serial(u->right, Rbase, t.out);
        }
    }
    return NULL;
}

static void seed_initial_tasks(Node *u, int base, int *out, int grain, int seeds){
    if (!u || seeds <= 1){
        FillTask *T = arena_alloc(&g_arena);
        if (!T) { fill_inorder_serial(u, base, out); return; }
        T->u = u; T->base = base; T->out = out; T->grain = grain;
        (void)thr_pool_queue(g_pool, fill_task_run, T);
        return;
    }
    out[base + u->leftNodeNum] = u->data;
    int left_seeds  = seeds / 2;
    int right_seeds = seeds - left_seeds;
    if (u->left)  seed_initial_tasks(u->left,  base, out, grain, left_seeds);
    if (u->right) seed_initial_tasks(u->right, base + u->leftNodeNum + 1, out, grain, right_seeds);
}

static void fill_inorder_parallel(Node *root, int *out, int grain, int nthreads, int total_nodes){
    if (!root) return;

    int max_tasks = (total_nodes / (grain ? grain : 1)) * 2 + (nthreads * 2) + 16;
    g_arena.buf  = (FillTask*)malloc((size_t)max_tasks * sizeof(FillTask));
    g_arena.cap  = max_tasks;
    atomic_store(&g_arena.next, 0);

    g_pool = thr_pool_create(nthreads, nthreads, /*linger*/2, /*attr*/NULL);

    int seeds = nthreads; /* start ≈ one task per worker */
    seed_initial_tasks(root, 0, out, grain, seeds);

    thr_pool_wait(g_pool);
    thr_pool_destroy(g_pool);
    g_pool = NULL;

    free(g_arena.buf);
    memset(&g_arena, 0, sizeof(g_arena));
}

/* ===== serial baseline: just traversal (no sizes, no fill) ===== */
static void inorder_traverse_serial(Node *u){
    if (!u) return;
    inorder_traverse_serial(u->left);
    /* no-op */
    inorder_traverse_serial(u->right);
}

/* ===== main ===== */
int main(int argc, char **argv){
    if (argc < 2){
        fprintf(stderr, "Usage: %s <N> [grain] [threads]\n", argv[0]);
        return 2;
    }
    long long N = atoll(argv[1]);
    if (N <= 0){ fprintf(stderr, "N must be > 0\n"); return 2; }

    int grain   = (argc >= 3) ? atoi(argv[2]) : 16384;
    int threads = (argc >= 4) ? atoi(argv[3]) : 0;
    if (threads <= 0){
        /* default to cores if not provided */
        long t = sysconf(_SC_NPROCESSORS_ONLN);
        threads = (t > 0) ? (int)t : 4;
    }

    Node *root_parallel = build_bst_sorted((int)N);
    Node *root_serial   = build_bst_sorted((int)N);

    /* Parallel path: serial sizes + parallel coarse fill */
    double tp0 = now_sec();
    int total = fill_sizes_serial(root_parallel);
    int *out = (int*)calloc((size_t)total, sizeof(int));
    double tp1 = now_sec();
    fill_inorder_parallel(root_parallel, out, grain, threads, total);
    printf("Parallel in-order — nodes=%d, threads=%d, grain=%d\n", total, threads, grain);
    printf("Parallel time (sizes + fill): %.6f sec\n\n", tp1 - tp0);

    /* Serial baseline: plain traversal (what you want to show) */
    double ts0 = now_sec();
    inorder_traverse_serial(root_serial);
    double ts1 = now_sec();
    printf("Serial in-order (just traversal) — nodes=%d\n", (int)N);
    printf("Serial traversal time: %.6f sec\n\n", ts1 - ts0);

   
    //for (int i = 0; i < total; ++i) printf("%d ", out[i]); printf("\n");
    
    free(out);
    return 0;
}
