#include "utils.h"

#define MAX_CHILD_NODES 5

typedef enum {
    FUNCTION_NODE = 1,

    FUNCTION_HEADER_NODE,
    EXPRESSION_PARAMETER_BY_REFERENCE_NODE,
    EXPRESSION_PARAMETER_BY_VALUE_NODE,
    FUNCTION_PARAMETER_NODE,

    BLOCK_NODE,
    LABELS_NODE,
    TYPES_NODE,
    VARIABLES_NODE,
    FUNCTIONS_NODE,
    BODY_NODE,

    TYPE_DECLARATION_NODE,
    DECLARATION_NODE,
    TYPE_NODE,
    ARRAY_SIZE_NODE,

    STATEMENT_NODE,
    LABEL_NODE,

    ASSIGNMENT_NODE,
    VALUE_NODE,
    ARRAY_INDEX_NODE,

    FUNCTION_CALL_NODE,

    GOTO_NODE,

    RETURN_NODE,

    IF_NODE,

    WHILE_NODE,

    COMPOUND_NODE,

    EXPRESSION_NODE,
    BINARY_OPERATOR_EXPRESSION_NODE,
    UNARY_OPERATOR_EXPRESSION_NODE,
    TERM_NODE,
    FACTOR_NODE,

    INTEGER_NODE,
    IDENTIFIER_NODE,

    RELATIONAL_OPERATOR_NODE,
    LESS_OR_EQUAL_NODE,
    LESS_NODE,
    EQUAL_NODE,
    DIFFERENT_NODE,
    GREATER_OR_EQUAL_NODE,
    GREATER_NODE,

    ADDITIVE_OPERATOR_NODE,
    UNARY_OPERATOR_NODE,
    PLUS_NODE,
    MINUS_NODE,
    OR_NODE,
    NOT_NODE,

    MULTIPLICATIVE_OPERATOR_NODE,
    MULTIPLY_NODE,
    DIV_NODE,
    AND_NODE
} NodeCategory;

typedef struct _treeNode {
    NodeCategory category;
    char *name;
    struct _treeNode *next;
    struct _treeNode *subtrees[MAX_CHILD_NODES];
} TreeNode, *TreeNodePtr;


void *getTree();
void counts(void *p, int *functions, int *funcalls, int *whiles, int *ifs, int *bin);

/**
 * Initializes a tree node considering that the "numberOfChildNodes" top elements on the stack
 * are the subtrees of the new node.
 **/
void addTreeNodeWithName(NodeCategory category, int numberOfChildNodes, char *name);
void addTreeNode(NodeCategory category, int numberOfChildNodes);

/**
 * A sequence is defined by the "next" attribute in a tree node, each subtree can have another subtree in its sequence
 * this is used by sequence of statements, sequence of identifiers, etc.
 **/
void addSequence();

void addEmpty();
void addIdentifier(char *tokenValue);

/**
 * It prints the syntax tree on standard output
 * Subtrees which form a sequence (ex: sequence of statements, sequence of identifiers, etc.) will be represented with
 * the prefix "|->" before all the elements on the sequence except the first one
 **/
void dumpTree(void *p);
const char *getCategoryName(NodeCategory category);
