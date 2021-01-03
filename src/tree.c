#include "tree.h"

#include "symboltable.h"
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
 * Tree Visualization functions
 */
void dumpSyntaxTree(TreeNodePtr node, int indent, bool isNext);
const char *getCategoryName(NodeCategory category);
void addIndent(int indent);



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

    for (int i = numberOfChildNodes-1; i >= 0; i--) {
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

void dumpTree(void *root) {
    TreeNodePtr tree = (TreeNodePtr) root;
    dumpSyntaxTree(tree, 0, false);
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

/**
 * Tree Visualization functions
 */
void dumpSyntaxTree(TreeNodePtr node, int indent, bool isNext) {
    if(node == NULL) {
        return;
    }

    addIndent(indent);
    if(isNext) {
        printf("|-> ");
    }

    if(node->name == NULL) {
        printf("%s\n", getCategoryName(node->category));
    } else {
        printf("%s(%s)\n", getCategoryName(node->category), node->name);
    }

    for (int i = MAX_CHILD_NODES-1; i >= 0; i--) {
        TreeNodePtr subtree = node->subtrees[i];
        dumpSyntaxTree(subtree, indent + 1, false);
    }

    dumpSyntaxTree(node->next, indent, true);

}

void addIndent(int indent) {
    for (int i = 0; i < indent; i++) {
        printf("\t");
    }
}

const char *getCategoryName(NodeCategory category) {
    switch (category) {
        case FUNCTION_NODE:
            return "FUNCTION_NODE";

        case FUNCTION_HEADER_NODE:
            return "FUNCTION_HEADER_NODE";
        case EXPRESSION_PARAMETER_BY_REFERENCE_NODE:
            return "EXPRESSION_PARAMETER_BY_REFERENCE_NODE";
        case EXPRESSION_PARAMETER_BY_VALUE_NODE:
            return "EXPRESSION_PARAMETER_BY_VALUE_NODE";
        case FUNCTION_PARAMETER_NODE:
            return "FUNCTION_PARAMETER_NODE";

        case BLOCK_NODE:
            return "BLOCK_NODE";
        case LABELS_NODE:
            return "LABELS_NODE";
        case TYPES_NODE:
            return "TYPES_NODE";
        case VARIABLES_NODE:
            return "VARIABLES_NODE";
        case FUNCTIONS_NODE:
            return "FUNCTIONS_NODE";
        case BODY_NODE:
            return "BODY_NODE";

        case TYPE_DECLARATION_NODE:
            return "TYPE_DECLARATION_NODE";
        case DECLARATION_NODE:
            return "DECLARATION_NODE";
        case TYPE_NODE:
            return "TYPE_NODE";
        case ARRAY_SIZE_NODE:
            return "ARRAY_SIZE_NODE";

        case STATEMENT_NODE:
            return "STATEMENT_NODE";
        case LABEL_NODE:
            return "LABEL_NODE";

        case ASSIGNMENT_NODE:
            return "ASSIGNMENT_NODE";
        case VALUE_NODE:
            return "VALUE_NODE";
        case ARRAY_INDEX_NODE:
            return "ARRAY_INDEX_NODE";

        case FUNCTION_CALL_NODE:
            return "FUNCTION_CALL_NODE";

        case GOTO_NODE:
            return "GOTO_NODE";

        case RETURN_NODE:
            return "RETURN_NODE";

        case IF_NODE:
            return "IF_NODE";

        case WHILE_NODE:
            return "WHILE_NODE";

        case COMPOUND_NODE:
            return "COMPOUND_NODE";

        case EXPRESSION_NODE:
            return "EXPRESSION_NODE";
        case BINARY_OPERATOR_EXPRESSION_NODE:
            return "BINARY_OPERATOR_EXPRESSION_NODE";
        case UNARY_OPERATOR_EXPRESSION_NODE:
            return "UNARY_OPERATOR_EXPRESSION_NODE";
        case TERM_NODE:
            return "TERM_NODE";
        case FACTOR_NODE:
            return "FACTOR_NODE";

        case INTEGER_NODE:
            return "INTEGER_NODE";
        case IDENTIFIER_NODE:
            return "IDENTIFIER_NODE";

        case RELATIONAL_OPERATOR_NODE:
            return "RELATIONAL_OPERATOR_NODE";
        case LESS_OR_EQUAL_NODE:
            return "LESS_OR_EQUAL_NODE";
        case LESS_NODE:
            return "LESS_NODE";
        case EQUAL_NODE:
            return "EQUAL_NODE";
        case DIFFERENT_NODE:
            return "DIFFERENT_NODE";
        case GREATER_OR_EQUAL_NODE:
            return "GREATER_OR_EQUAL_NODE";
        case GREATER_NODE:
            return "GREATER_NODE";

        case ADDITIVE_OPERATOR_NODE:
            return "ADDITIVE_OPERATOR_NODE";
        case UNARY_OPERATOR_NODE:
            return "UNARY_OPERATOR_NODE";
        case PLUS_NODE:
            return "PLUS_NODE";
        case MINUS_NODE:
            return "MINUS_NODE";
        case OR_NODE:
            return "OR_NODE";
        case NOT_NODE:
            return "NOT_NODE";

        case MULTIPLICATIVE_OPERATOR_NODE:
            return "MULTIPLICATIVE_OPERATOR_NODE";
        case MULTIPLY_NODE:
            return "MULTIPLY_NODE";
        case DIV_NODE:
            return "DIV_NODE";
        case AND_NODE:
            return "AND_NODE";
    }
}