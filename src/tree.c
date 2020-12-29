#include "tree.h"

#include <stdlib.h>
#include <stdio.h>

/**
 * Internal variables and functions declaration
 **/

/**
 * Always use getStack() to get the stack, it will ensure that the stack is initialized
 **/
Stack *stack = NULL;
Stack *getStack();

int count(TreeNodePtr treeNodePtr, NodeCategory category);


/**
 * Public functions implementation
 **/

/**
 * The syntax tree will be on top of the stack at the end of the parsing phase
 * This function should be called only after the parser has finished
 * If there is more than one element on the stack, this function will terminate the program with error
 **/
void *getTree() {
    Stack *stack = getStack();

    if (stack->size > 1) {
        fprintf(stderr, "Stack should have only one element which is the syntax tree root, but it has %d elements", stack->size);
        exit(EXIT_FAILURE);
    }

    TreeNodePtr syntaxTree = (TreeNodePtr) pop(stack);

    free(stack);

    return syntaxTree;
}

void counts(void *p, int *functions, int *funcalls, int *whiles, int *ifs, int *bin) {
    TreeNodePtr treeNodePtr = (TreeNodePtr) p;

    *functions = count(treeNodePtr, FUNCTION_NODE);
    *funcalls = count(treeNodePtr, FUNCTION_CALL_NODE);
    *whiles = count(treeNodePtr, WHILE_NODE);
    *ifs = count(treeNodePtr, IF_NODE);

    int relationalExpressions = count(treeNodePtr, RELATIONAL_OPERATOR_NODE);
    int additiveExpressions = count(treeNodePtr, ADDITIVE_OPERATOR_NODE);
    int multiplicativeExpressions = count(treeNodePtr, MULTIPLICATIVE_OPERATOR_NODE);
    *bin = relationalExpressions + additiveExpressions + multiplicativeExpressions;
}

void addTreeNodeWithName(NodeCategory category, int numberOfChildNodes, char *name) {

    TreeNodePtr node = malloc(sizeof(TreeNode));
    node->category = category;
    node->name = name;
    node->next = NULL;

    Stack *stack = getStack();

    for (int i = 0; i < numberOfChildNodes; i++) {
        TreeNodePtr childNode = pop(stack);
        node->subtrees[i] = childNode;
    }

    for (int i = numberOfChildNodes; i < MAX_CHILD_NODES; i++) {
        node->subtrees[i] = NULL;
    }

    push(stack, node);
}

void addTreeNode(NodeCategory category, int numberOfChildNodes) {
    addTreeNodeWithName(category, numberOfChildNodes, NULL);
}

void addSequence() {
    Stack *stack = getStack();

    TreeNodePtr topNode = pop(stack);
    TreeNodePtr nextNode = pop(stack);

    if (nextNode != NULL) {
        nextNode->next = topNode;
        push(stack, nextNode);
    } else {
        push(stack, topNode);
    }
}

void addEmpty() {
    Stack *stack = getStack();
    push(stack, NULL);
}

void addIdentifier(char *tokenValue) {
    addTreeNodeWithName(IDENTIFIER_NODE, 0, tokenValue);
}

/**
 * Internal functions implementation
 **/

int count(TreeNodePtr treeNodePtr, NodeCategory category) {
    if (treeNodePtr == NULL) {
        return 0;
    }

    int nodesWithCategory = 0;
    if(treeNodePtr->category == category) {
        nodesWithCategory++;
    }

    nodesWithCategory += count(treeNodePtr->next, category);

    for (int i = 0; i < MAX_CHILD_NODES; i++) {
        TreeNodePtr child = treeNodePtr->subtrees[i];
        nodesWithCategory += count(child, category);
    }

    return nodesWithCategory;
}

Stack *getStack() {
    if (stack == NULL) {
        stack = newStack();
    }
    return stack;
}