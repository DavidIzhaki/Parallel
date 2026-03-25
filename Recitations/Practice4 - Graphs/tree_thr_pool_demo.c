#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <pthread.h>
#include <time.h>
#include "thr_pool.h"

/* ============================================================
   Timing helper (monotonic wall-clock)
   ============================================================ */
static inline double now_sec(void){
    struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

/* ============================================================
   Tree node
   - parent pointer (set in init)
   - left/right subtree sizes (computed bottom-up)
   - pendingChildren: atomic countdown latch (0..2)
   ============================================================ */
typedef struct Node {
    int data;
    struct Node *left, *right, *parent;
    int leftNodeNum, rightNodeNum;
    _Atomic int pendingChildren;
} Node;

/* ============================================================
   Build a balanced BST with keys 1..N
   (so in-order is naturally sorted)
   ============================================================ */
static Node* new_node(int x){ Node *n = calloc(1,sizeof(Node)); n->data = x; return n; }

static Node* build_balanced_range(int lo, int hi){
    if (lo > hi) return NULL;
    int mid = lo + (hi - lo)/2;
    Node *u = new_node(mid);
    u->left  = build_balanced_range(lo, mid-1);
    u->right = build_balanced_range(mid+1, hi);
    return u;
}

static Node* build_bst_sorted(int N){ return build_balanced_range(1, N); }

/* ============================================================
   GLOBAL: thread pool handle (we reuse the same name in phases)
   ============================================================ */
static thr_pool_t *pool = NULL;

/* ============================================================
   PHASE A: Bottom-up (parallel) with thread pool
   - Idea: a node is "ready" when both children finished.
   - Mechanism:
       pendingChildren = (#non-null children)
       leaves -> pendingChildren==0 -> queue as tasks
       when child finishes: parent.pendingChildren-- (atomic)
       if becomes 0 -> enqueue parent
   ============================================================ */
static int subtree_size(Node *u){ return 1 + u->leftNodeNum + u->rightNodeNum; }

/* Worker task: process a READY node v and notify its parent */
static void *process_node(void *arg){
    Node *v = (Node*)arg;
    Node *p = v->parent;

    if (p){
        /* Each child writes a different field (no race): */
        int sz = subtree_size(v);
        if (p->left == v)  p->leftNodeNum  = sz;
        else               p->rightNodeNum = sz;

        /* Atomic countdown: only the last child enqueues the parent */
        if (atomic_fetch_sub(&p->pendingChildren, 1) == 1){
            /* Parent is now READY */
            (void)thr_pool_queue(pool, process_node, p);
        }
    }
    return NULL;
}

/* One-time initialization:
   - set parent pointers
   - init subtree counters to 0
   - set pendingChildren to #children
   - enqueue leaves (pendingChildren==0) into the pool
   Traversal is serial; work itself is parallel after this. */
static void init_dfs(Node *u, Node *p){
    if (!u) return;
    u->parent = p;
    u->leftNodeNum = u->rightNodeNum = 0;

    int c = (u->left ? 1 : 0) + (u->right ? 1 : 0);
    atomic_store(&u->pendingChildren, c);

    init_dfs(u->left,  u);
    init_dfs(u->right, u);

    if (c == 0) {
        /* Leaf is READY immediately */
        (void)thr_pool_queue(pool, process_node, u);
    }
}

/* Compute sizes using the thread pool */
static void compute_sizes_parallel(Node *root, int min_thr, int max_thr){
    if (!root) return;
    pool = thr_pool_create(min_thr, max_thr, /*linger*/0, /*attr*/NULL);

    init_dfs(root, NULL);     /* seeds the work by queuing leaves */
    thr_pool_wait(pool);      /* wait until all ready-tasks drain upward */
    thr_pool_destroy(pool);
    pool = NULL;
}

/* ============================================================
   PHASE B: Top-down (parallel) fill array by in-order index
   - index(u) = base(u) + leftNodeNum(u)
   - base(left)  = base(u)
   - base(right) = base(u) + leftNodeNum(u) + 1
   We use the thread pool again; each task writes one out[idx].
   ============================================================ */
typedef struct {
    Node *u;
    int base;
    int *out;
} TopTaskArg;

static void *topdown_task(void *arg){
    TopTaskArg *ta = (TopTaskArg*)arg;
    Node *u = ta->u;
    int base = ta->base;
    int *out = ta->out;
    free(ta); /* allocated per task */

    /* Place this node's value at its in-order index */
    int idx = base + u->leftNodeNum;
    out[idx] = u->data;

    /* Enqueue children (independent subtrees write disjoint ranges) */
    if (u->left){
        TopTaskArg *L = malloc(sizeof(*L));
        L->u = u->left; L->base = base; L->out = out;
        (void)thr_pool_queue(pool, topdown_task, L);
    }
    if (u->right){
        TopTaskArg *R = malloc(sizeof(*R));
        R->u = u->right; R->base = base + u->leftNodeNum + 1; R->out = out;
        (void)thr_pool_queue(pool, topdown_task, R);
    }
    return NULL;
}

static void fill_inorder_array_parallel(Node *root, int *out, int min_thr, int max_thr){
    if (!root) return;
    pool = thr_pool_create(min_thr, max_thr, 0, NULL);

    TopTaskArg *A = malloc(sizeof(*A));
    A->u = root; A->base = 0; A->out = out;
    (void)thr_pool_queue(pool, topdown_task, A);

    thr_pool_wait(pool);
    thr_pool_destroy(pool);
    pool = NULL;
}

/* ============================================================
   Serial baseline: just print the values in-order (no metadata)
   ============================================================ */
static void inorder_print_serial(Node *u){
    if (!u) return;
    inorder_print_serial(u->left);
    // printf("%d ", u->data);
    inorder_print_serial(u->right);
}

/* ============================================================
   Main demo
   ============================================================ */
int main(int argc, char **argv){
    /* Choose a size students can see but is non-trivial.
       You can bump to 511 / 1023 for larger trees. */

     char *end = NULL;
    unsigned long long N = strtoull(argv[1], &end, 10);
 

    /* Build two identical trees: one for parallel, one for serial baseline */
    Node *root_par = build_bst_sorted(N);
    Node *root_ser = build_bst_sorted(N);

    /* ---------------- PARALLEL: sizes (bottom-up) + array fill (top-down) ---------------- */
    double tp0 = now_sec();
    compute_sizes_parallel(root_par, /*min*/4, /*max*/8);

    /* After bottom-up, total node count = 1 + left(root) + right(root) */
    int total = 1 + root_par->leftNodeNum + root_par->rightNodeNum;
    int *out = calloc(total, sizeof(int));

    fill_inorder_array_parallel(root_par, out, /*min*/4, /*max*/8);
    double tp1 = now_sec();

    /* Print the final ordered sequence once (stdout is inherently serial) */
    printf("Parallel in-order (by array fill) — %d nodes:\n", total);
    //for (int i = 0; i < total; ++i) printf("%d ", out[i]);
    printf("\nParallel compute time (sizes + fill): %.6f sec\n\n", tp1 - tp0);
    free(out);

    /* ---------------- SERIAL: just in-order traversal print ---------------- */
    double ts0 = now_sec();
    printf("Serial in-order (direct traversal) — %d nodes:\n", N);
    inorder_print_serial(root_ser);
    double ts1 = now_sec();
    printf("\nSerial traversal time: %.6f sec\n\n", ts1 - ts0);

    /* IMPORTANT NOTE for students:
       - Parallel "compute time" above excludes print cost (we parallelized placement, not printf).
       - Serial baseline prints as it traverses (printf dominates).
       For apples-to-apples *compute* comparison, measure without printing or fill an array serially. */

    return 0;
}
