/**
 * This module provides functionality to use generic data structures
 **/

#ifndef DATA_STRUCTURES_HEADER
#define DATA_STRUCTURES_HEADER

typedef enum {false, true} bool;

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
void* find(Stack* stack, void* secondParam, bool (*predicate)(void*, void*));

#endif