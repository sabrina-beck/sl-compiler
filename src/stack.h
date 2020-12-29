/**
 * This module provides functionality to use a stack as data structure
 **/

typedef struct _stackNode {
    void* data;
    struct _stackNode* next;
} StackNode;

typedef struct _stack {
    StackNode* top;
    int size ;
} Stack;

Stack* newStack();
void push(Stack* stack, void* data);
void* pop(Stack* stack);