#include "datastructures.h"

#include <stdlib.h>

/* Auxiliary functions for memory allocation and deallocation of the stack nodes */
LinkedNode* newLinkedNode(void* data, LinkedNode* next);
void freeLinkedNode(LinkedNode* node);

/**
 * Public definitions implementation for Stack
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
 * Public definitions implementation for Queue
 **/

Queue* newQueue() {
    Queue* queue = malloc(sizeof(Queue));
    queue->size = 0;
    queue->front = NULL;
    queue->rear = NULL;
    return queue;
}

void enqueue(Queue* queue, void* data) {
    LinkedNode newNode = newLinkedNode(data, NULL);
    queue->size++;

    if(queue->rear == NULL) {
        queue->front = newNode;
        queue->rear = newNode;
    } else {
        queue->rear->next = newNode;
        queue->rear = newNode;
    }
}

void* dequeue(Queue* queue){
    if(queue->front == NULL) {
        return NULL;
    }

    LinkedNode dequeuedNode = queue->front;
    queue->size--;
    queue->front = dequeuedNode->next;

    void* dequeuedData = dequeuedNode.data;
    freeLinkedNode(dequeuedNode);
    return dequeuedData;
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