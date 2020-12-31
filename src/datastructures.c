#include "datastructures.h"

#include <stdlib.h>

/* Auxiliary functions for memory allocation and deallocation of the stack nodes */
LinkedNode* newLinkedNode(void* data, LinkedNode* next);
void freeLinkedNode(LinkedNode* node);

/**
 * Public definitions implementation
 **/
Stack* newStack() {
    Stack* stack = malloc(sizeof(Stack));
    stack->size = 0;
    stack->top = NULL;
    return stack;
}

void push(Stack* stack, void* data) {
    LinkedNode* newNode = newLinkedNode(data, stack->top);
    stack->top = newNode;
    stack->size++;
}

void* pop(Stack* stack){
    LinkedNode* poppedNode = stack->top;
    stack->top = poppedNode->next;
    stack->size--;

    void* data = poppedNode->data;
    freeLinkedNode(poppedNode);

    return data;
}

/**
 * Private definitions implementation
 **/
LinkedNode* newLinkedNode(void* data, LinkedNode* next) {
    LinkedNode* newNode = malloc(sizeof(LinkedNode));
    newNode->data = data;
    newNode->next = next;
    return newNode;
}

void freeLinkedNode(LinkedNode* node) {
    free(node);
}