/**
 * This module provides functionality to use generic data structures
 **/

#ifndef DATA_STRUCTURES_HEADER
#define DATA_STRUCTURES_HEADER

/***********************************************************************************************************************
 * Boolean definition
 **********************************************************************************************************************/
typedef enum {false, true} bool;

/***********************************************************************************************************************
 * Stack
 **********************************************************************************************************************/
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

/***********************************************************************************************************************
 * Code generation functions
 **********************************************************************************************************************/

void addCommand(const char* commandFormat, ...);

#endif