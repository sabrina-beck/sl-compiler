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

typedef struct _list {
    LinkedNode* front;
    int size;
} List;
List* newList();
void add(List* list, void* data);
List* concat(List* list1, List* list2);

typedef struct _stack {
    LinkedNode* top;
    int size ;
} Stack;

Stack* newStack();
void push(Stack* stack, void* data);
void* pop(Stack* stack);
void* find(Stack* stack, void* secondParam, bool (*predicate)(void*, void*));

typedef struct _queue {
    LinkedNode* front;
    LinkedNode* rear;
    int size ;
} Queue;

Queue* newQueue();
void enqueue(Queue* queue, void* data);
void* dequeue(Queue* queue);

#endif