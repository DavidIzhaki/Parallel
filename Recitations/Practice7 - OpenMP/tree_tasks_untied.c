#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

typedef struct Node {
    int value;
    struct Node *left;
    struct Node *right;
} Node;

/* Do some work on a node */
void Process(Node *n) {
    printf("Processing value %d on thread %d\n",
           n->value, omp_get_thread_num());
}

/* Recursive traversal using OpenMP tasks (post-order) */
void Traverse(Node *n) {
    if (n == NULL) return;

    if (n->left != NULL) {
        Node *child = n->left;
        #pragma omp task firstprivate(child) untied
        {
            Traverse(child);
        }
    }

    if (n->right != NULL) {
        Node *child = n->right;
        #pragma omp task firstprivate(child) untied
        {
            Traverse(child);
        }
    }

    #pragma omp taskwait          // wait for left/right subtasks
    Process(n);                    // then process this node
}

/* Start traversal with an OpenMP parallel region */
void Travel(Node *root) {
    #pragma omp parallel
    {
        #pragma omp single
        {
            Traverse(root);        // only one thread starts recursion
        }

        #pragma omp taskwait       // make sure all tasks are done
    }
}

/* Helper to build nodes */
Node *new_node(int value, Node *left, Node *right) {
    Node *n = malloc(sizeof(Node));
    n->value = value;
    n->left  = left;
    n->right = right;
    return n;
}

int main(void) {
    omp_set_num_threads(4);

    /* Build a small tree:
             1
           /   \
          2     3
         / \   / \
        4  5  6   7
    */
    Node *n4 = new_node(4, NULL, NULL);
    Node *n5 = new_node(5, NULL, NULL);
    Node *n6 = new_node(6, NULL, NULL);
    Node *n7 = new_node(7, NULL, NULL);
    Node *n2 = new_node(2, n4, n5);
    Node *n3 = new_node(3, n6, n7);
    Node *n1 = new_node(1, n2, n3);   // root

    Travel(n1);

    /* (No free() here to keep the example short) */
    return 0;
}
