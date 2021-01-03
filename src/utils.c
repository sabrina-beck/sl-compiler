#include "utils.h"
#include "slc.h"

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

/***********************************************************************************************************************
 * Stack
 **********************************************************************************************************************/

/* Private declarations */
LinkedNode* newLinkedNode(void* data, LinkedNode* next);
void freeLinkedNode(LinkedNode* node);

/* Implementation */
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
        current = current->next;
    }

    return NULL;
}

/* Private Implementations */
LinkedNode* newLinkedNode(void* data, LinkedNode* next) {
    LinkedNode* newNode = malloc(sizeof(LinkedNode));
    newNode->data = data;
    newNode->next = next;
    return newNode;
}

void freeLinkedNode(LinkedNode* node) {
    free(node);
}

/***********************************************************************************************************************
 * Code generation functions
 **********************************************************************************************************************/

void addCommand(const char* commandFormat, ...) {
    va_list args;
    va_start(args, commandFormat);
    printf("\t");
    vprintf(commandFormat, args);
    printf("\n");
    va_end(args);
}

/***********************************************************************************************************************
 * Semantic Error Treatment
 **********************************************************************************************************************/

void throwSemanticError(const char* messageFormat, ...) {
    va_list args;
    va_start(args, messageFormat);

    char message[500];
    vsprintf(message, messageFormat, args);
    SemanticError(message);

    va_end(args);
}