#include "tree_visualizer.h"
#include <stdio.h>

typedef enum {false, true} bool;

void dumpSyntaxTree(TreeNodePtr node, int indent, bool isNext);
const char *getCategoryName(NodeCategory category);
void addIndent(int indent);

/**
 * Public declarations implementation
 **/

void dumpTree(void *root) {
    TreeNodePtr tree = (TreeNodePtr) root;
    dumpSyntaxTree(tree, 0, false);
}

/**
 * Private declarations implementation
 **/
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
        case VARIABLE_NODE:
            return "VARIABLE_NODE";
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
        case ADDITIVE_OPERATION_NODE:
            return "ADDITIVE_OPERATION_NODE";
        case TERM_NODE:
            return "TERM_NODE";
        case MULTIPLICATIVE_OPERATION_NODE:
            return "MULTIPLICATIVE_OPERATION_NODE";
        case FACTOR_NODE:
            return "FACTOR_NODE";

        case INTEGER_NODE:
            return "INTEGER_NODE";
        case IDENTIFIER_NODE:
            return "IDENTIFIER_NODE";

        case RELATIONAL_OPERATOR_NODE:
            return "RELATIONAL_OPERATOR_NODE";
        case ADDITIVE_OPERATOR_NODE:
            return "ADDITIVE_OPERATOR_NODE";
        case UNARY_OPERATOR_NODE:
            return "UNARY_OPERATOR_NODE";
        case MULTIPLICATIVE_OPERATOR_NODE:
            return "MULTIPLICATIVE_OPERATOR_NODE";
    }
}