#include "codegen.h"
#include "tree.h"
#include <stdio.h>
#include "utils.h"

void processAssignment(TreeNodePtr node);

// ...

void processExpression(TreeNodePtr node);
void routeExpressionSubtree(TreeNodePtr node);
void processBinaryOpExpression(TreeNodePtr node);
void processUnopExpression(TreeNodePtr node);
void processAdditiveOperation(TreeNodePtr node);
void processTerm(TreeNodePtr node);
void processMultiplicativeOperation(TreeNodePtr node);
void processFactor(TreeNodePtr node);
void processInteger(TreeNodePtr node);
void processRelationalOperator(TreeNodePtr node);
void processAdditiveOperator(TreeNodePtr node);
void processUnaryOperator(TreeNodePtr node);
void processMultiplicativeOperator(TreeNodePtr node);


void processProgram(void *p) {
    TreeNodePtr treeRoot = (TreeNodePtr) p;
    processExpression(treeRoot);
}

void processAssignment(TreeNodePtr node) {
    if(node->category != ASSIGNMENT_NODE) {
        fprintf(stderr, "Expected assignment node!\n");
        return;
    }

    TreeNodePtr variableNode = node->subtrees[0];
    TreeNodePtr expressionNode = node->subtrees[1];

    processExpression(expressionNode);
    // TODO processVariable
    // TODO STVl
}

// ...

void processExpression(TreeNodePtr node) {
    if(node->category != EXPRESSION_NODE) {
        fprintf(stderr, "Expected expression node!\n");
        return;
    }

    TreeNodePtr childExprNode = node->subtrees[0];
    TreeNodePtr relationalOperatorNode = node->subtrees[1];
    TreeNodePtr binaryopExprNode = node->subtrees[2];

    routeExpressionSubtree(childExprNode);
    processBinaryOpExpression(binaryopExprNode);
    processRelationalOperator(relationalOperatorNode);
}

void routeExpressionSubtree(TreeNodePtr node) {
    switch (node->category) {
        case BINARY_OPERATOR_EXPRESSION_NODE:
            processBinaryOpExpression(node);
        break;
        case UNARY_OPERATOR_EXPRESSION_NODE:
            processUnopExpression(node);
        break;
        default:
            fprintf(stderr, "Expected sub expression node!\n");
    }
}

void processBinaryOpExpression(TreeNodePtr node) {
    if(node->category != BINARY_OPERATOR_EXPRESSION_NODE) {
        fprintf(stderr, "Expected binary operator expression node!\n");
        return;
    }

    TreeNodePtr termNode = node->subtrees[0];
    TreeNodePtr additiveOperationNode = node->subtrees[1];

    processTerm(termNode);
    processAdditiveOperation(additiveOperationNode);
}

void processUnopExpression(TreeNodePtr node) {
    if(node->category != UNARY_OPERATOR_EXPRESSION_NODE) {
        fprintf(stderr, "Expected unary operator expression node!\n");
        return;
    }

    TreeNodePtr unaryOperatorNode = node->subtrees[0];
    TreeNodePtr termNode = node->subtrees[1];
    TreeNodePtr additiveOperationNode = node->subtrees[2];

    processTerm(termNode);
    processUnaryOperator(unaryOperatorNode);
    processAdditiveOperation(additiveOperationNode);
}

void processAdditiveOperation(TreeNodePtr node) {
    if(node == NULL) {
        return;
    }

    if(node->category != ADDITIVE_OPERATION_NODE) {
        fprintf(stderr, "Expected additive operation node!\n");
        return;
    }

    TreeNodePtr operatorNode = node->subtrees[0];
    TreeNodePtr termNode = node->subtrees[1];
    TreeNodePtr additiveOperationNode = node->subtrees[2];

    processTerm(termNode);
    processAdditiveOperator(operatorNode);
    processAdditiveOperation(additiveOperationNode);
}

void processTerm(TreeNodePtr node) {
    if(node->category != TERM_NODE) {
        fprintf(stderr, "Expected term node!\n");
        return;
    }

    TreeNodePtr factorNode = node->subtrees[0];
    TreeNodePtr multiplicativeOperationNode = node->subtrees[1];

    processFactor(factorNode);
    processMultiplicativeOperation(multiplicativeOperationNode);
}

void processMultiplicativeOperation(TreeNodePtr node) {
    if(node == NULL) {
        return;
    }

    if(node->category != MULTIPLICATIVE_OPERATION_NODE) {
        fprintf(stderr, "Expected multiplicative operation node!\n");
        return;
    }

    TreeNodePtr operatorNode = node->subtrees[0];
    TreeNodePtr factorNode = node->subtrees[1];
    TreeNodePtr multiplicativeOperationNode = node->subtrees[2];

    processFactor(factorNode);
    processMultiplicativeOperator(operatorNode);
    processMultiplicativeOperation(multiplicativeOperationNode);
}

void processFactor(TreeNodePtr node) {
    if(node->category != FACTOR_NODE) {
        fprintf(stderr, "Expected factor node!\n");
        return;
    }

    TreeNodePtr specificFactor = node->subtrees[0];
    switch (specificFactor->category) {
        case VARIABLE_NODE:
            //TODO processVariable();
            //TODO LDVL
        break;
        case INTEGER_NODE:
            processInteger(specificFactor);
        break;
        case FUNCTION_CALL_NODE:
            //TODO processFunctionCall();
        break;
        case EXPRESSION_NODE:
            processExpression(node);
        break;
        default:
            fprintf(stderr, "Unknown factor category"); // Semantic Error
    }
}


void processInteger(TreeNodePtr node) {
    if(node->category != INTEGER_NODE) {
        fprintf(stderr, "Expected integer node!\n");
        return;
    }

    char* integer = node->name;
    printf("LDCT %s\n", integer);
}


void processRelationalOperator(TreeNodePtr node) {
    if(node->category != RELATIONAL_OPERATOR_NODE) {
        fprintf(stderr, "Expected relational operator node!\n");
        return;
    }

    TreeNodePtr operatorNode = node->subtrees[0];
    if(operatorNode->category == LESS_OR_EQUAL_NODE) {
        printf("LEQU\n");
    } else if(operatorNode->category == LESS_NODE) {
        printf("LESS\n");
    } else if(operatorNode->category == EQUAL_NODE) {
        printf("EQUA\n");
    } else if(operatorNode->category == DIFFERENT_NODE) {
        printf("DIFF\n");
    } else if(operatorNode->category == GREATER_OR_EQUAL_NODE) {
        printf("GEQU\n");
    } else if(operatorNode->category == GREATER_NODE) {
        printf("GRTR\n");
    } else {
        fprintf(stderr, "Unknown relational operator"); // Semantic Error
    }
}

void processAdditiveOperator(TreeNodePtr node) {
    if(node->category != ADDITIVE_OPERATOR_NODE) {
        fprintf(stderr, "Expected additive operator node!\n");
        return;
    }

    TreeNodePtr operatorNode = node->subtrees[0];
    if(operatorNode->category == PLUS_NODE) {
        printf("ADDD\n");
    } else if(operatorNode->category == MINUS_NODE) {
        printf("SUBT\n");
    } else if(operatorNode->category == OR_NODE) {
        printf("LORR\n");
    } else {
        fprintf(stderr, "Unknown additive operator"); // Semantic Error
    }
}

void processUnaryOperator(TreeNodePtr node) {
    if(node->category != UNARY_OPERATOR_NODE) {
        fprintf(stderr, "Expected unary operator node!\n");
        return;
    }

    TreeNodePtr operatorNode = node->subtrees[0];
    if(operatorNode->category == PLUS_NODE) {
        // this operator has no practical effect
    } else if(operatorNode->category == MINUS_NODE) {
        printf("NEGT\n");
    } else if(operatorNode->category == NOT_NODE) {
        printf("LNOT\n");
    } else {
        fprintf(stderr, "Unknown unary operator"); // Semantic Error
    }
}

void processMultiplicativeOperator(TreeNodePtr node) {
    if(node->category != MULTIPLICATIVE_OPERATOR_NODE) {
        fprintf(stderr, "Expected multiplicative operator node!\n");
        return;
    }

    TreeNodePtr operatorNode = node->subtrees[0];
    if(operatorNode->category == MULTIPLY_NODE) {
        printf("MULT\n");
    } else if(operatorNode->category == DIV_NODE) {
        printf("DIVI\n");
    } else if(operatorNode->category == AND_NODE) {
        printf("LAND\n");
    } else {
        fprintf(stderr, "Unknown multiplicative operator"); // Semantic Error
    }
}