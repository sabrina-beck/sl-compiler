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

/*
 * Creates new stack pointer
 */
Stack* newStack();

/*
 * Pushes an item with data on top of the stack
 */
void push(Stack* stack, void* data);

/*
 * Pops an items data from the top of the stack
 */
void* pop(Stack* stack);

/*
 * Finds the first occurrence of a item in which the predicate function returns true
 * The predicate function has as its first argument the item's data and the second argument is from the user's choice
 * used to calculate if the item is the desired one
 * SecondParam is the second parameter passed to the predicate.
 */
void* find(Stack* stack, void* secondParam, bool (*predicate)(void*, void*));

/***********************************************************************************************************************
 * Code generation functions
 **********************************************************************************************************************/
/*
 * Adds a new Mepa command to the output
 */
void addCommand(const char* commandFormat, ...);


/***********************************************************************************************************************
 * Semantic Error Treatment
 **********************************************************************************************************************/
/*
 * Throws a semantic error
 */
void throwSemanticError(const char* messageFormat, ...);

#endif