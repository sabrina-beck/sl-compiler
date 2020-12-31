/**
 * This module provides functionality to use generic data structures
 **/

typedef struct _linkedNode {
    void* data;
    struct _linkedNode* next;
} LinkedNode;

typedef struct _stack {
    LinkedNode* top;
    int size ;
} Stack;

Stack* newStack();
void push(Stack* stack, void* data);
void* pop(Stack* stack);
