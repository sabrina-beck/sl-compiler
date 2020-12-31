#include "datastructures.h"

#include <stdlib.h>

/** Auxiliary functions for memory allocation and deallocation of the stack nodes **/
LinkedNode* newLinkedNode(void* data, LinkedNode* next);
void freeLinkedNode(LinkedNode* node);

/**
 * Public definitions implementation for List
 **/
List* newList() {
    List* list = malloc(sizeof(List));
    list->size=0;
    list->front = NULL;
    return list;
}

void add(List* list, void* data) {
    LinkedNode* newNode = newLinkedNode(data, list->front);
    list->front = newNode;
    list->size++;
}

List* concat(List* list1, List* list2) {
    if(list1 == NULL) {
        List* list = newList();
        list->front = list2->front;
        list->size = list2->size;
        return list;
    }

    LinkedNode* current = list1->front;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = list2->front;
    list1->size = list1->size + list2->size;
}

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
    if(stack->top == NULL) {
        return NULL;
    }

    LinkedNode* poppedNode = stack->top;
    stack->top = poppedNode->next;
    stack->size--;

    void* data = poppedNode->data;
    freeLinkedNode(poppedNode);

    return data;
}

void* find(Stack* stack, void* secondParam, bool (*predicate)(void*, void*)) {
    LinkedNode* current = stack->top;
    while (current != NULL) {
        void* data = current->data;
        if(predicate(data, secondParam)) {
            return data;
        }
    }

    return NULL;
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
    LinkedNode* newNode = newLinkedNode(data, NULL);
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

    LinkedNode* dequeuedNode = queue->front;
    queue->size--;
    queue->front = dequeuedNode->next;

    void* dequeuedData = dequeuedNode->data;
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