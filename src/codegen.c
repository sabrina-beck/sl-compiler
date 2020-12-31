#include "codegen.h"
#include "tree.h"
#include "slc.h"
#include "utils.h"

#include <stdio.h>
#include <stdarg.h>

// TODO don't forget read / write predefined functions

void processFunction(TreeNodePtr node, bool mainFunction);

// ...

void processUnlabeledStatement(TreeNodePtr node);
void processAssignment(TreeNodePtr node);

// ...
void processConditional(TreeNodePtr node);
void processRepetitive(TreeNodePtr node);

void processCompound(TreeNodePtr node);
void processUnlabeledStatementList(TreeNodePtr node);

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

/** Error Handling **/
void UnexpectedNodeCategoryError(NodeCategory expected, NodeCategory gotten);
void UnexpectedChildNodeCategoryError(NodeCategory fatherNodeCategory, NodeCategory childNodeCategory);

/** Queue with the final program **/
Queue *programQueue = NULL;
void addCommand(const char* commandFormat, ...);
void printProgram();

void processProgram(void *p) {
    TreeNodePtr treeRoot = (TreeNodePtr) p;

    processFunction(treeRoot, true);

    addCommand("END ");

    printProgram();
}

void processFunction(TreeNodePtr node, bool mainFunction) {
    if(mainFunction) {
        addCommand("MAIN");
    }

    // [...]

    if(mainFunction) {
        addCommand("STOP");
    }
}

void processUnlabeledStatement(TreeNodePtr node) {
    if(node == NULL) { // treats empty statement
        return;
    }

    switch (node->category) {
        case ASSIGNMENT_NODE:
            processAssignment(node);
        break;
        case FUNCTION_CALL_NODE:
            // TODO processFunctionCall(node);
        break;
        case GOTO_NODE:
            // TODO processGoto(node);
        break;
        case RETURN_NODE:
            // TODO processReturn(node);
        break;
        case IF_NODE:
            processConditional(node);
        break;
        case WHILE_NODE:
            processRepetitive(node);
        break;
        case COMPOUND_NODE:
            processCompound(node);
        break;
        default:
            UnexpectedChildNodeCategoryError(STATEMENT_NODE, node->category);
    }
}

void processAssignment(TreeNodePtr node) {
    if(node->category != ASSIGNMENT_NODE) {
        UnexpectedNodeCategoryError(ASSIGNMENT_NODE, node->category);
    }

    TreeNodePtr variableNode = node->subtrees[0];
    TreeNodePtr expressionNode = node->subtrees[1];

    processExpression(expressionNode);
    // TODO processVariable
    // TODO STVl
}

// ...

void processConditional(TreeNodePtr node) {
    if(node->category != IF_NODE) {
        UnexpectedNodeCategoryError(IF_NODE, node->category);
    }

    TreeNodePtr conditionNode = node->subtrees[0];
    TreeNodePtr ifCompound = node->subtrees[1];
    TreeNodePtr elseCompound = node->subtrees[2];

    char* elseLabel = nextMEPALabel();
    char* elseExitLabel = nextMEPALabel();

    processExpression(conditionNode);
    addCommand("JUMPF %s", elseLabel);

    processCompound(ifCompound);

    if(elseCompound != NULL) {
        addCommand("JUMP %s", elseExitLabel);

        addCommand("%s: NOOP", elseLabel);
        processCompound(elseCompound);

        addCommand("%s: NOOP", elseExitLabel);
    } else {
        addCommand("%s: NOOP", elseLabel);
    }
}

void processRepetitive(TreeNodePtr node) {
    if(node->category != WHILE_NODE) {
        UnexpectedNodeCategoryError(WHILE_NODE, node->category);
    }

    TreeNodePtr conditionNode = node->subtrees[0];
    TreeNodePtr compoundNode = node->subtrees[1];

    char* conditionLabel = nextMEPALabel(); // L1
    char* exitLabel = nextMEPALabel(); // L2

    addCommand("%s: NOOP", conditionLabel);
    processExpression(conditionNode);
    addCommand("JUMPF %s", exitLabel);

    processCompound(compoundNode);
    addCommand("JUMP %s", conditionLabel);

    addCommand("%s: NOOP", exitLabel);

}

void processCompound(TreeNodePtr node) {
    if(node->category != COMPOUND_NODE) {
        UnexpectedNodeCategoryError(COMPOUND_NODE, node->category);
        return;
    }

    TreeNodePtr unlabeledStatementNodeList = node->subtrees[0];
    processUnlabeledStatementList(unlabeledStatementNodeList);
}

void processUnlabeledStatementList(TreeNodePtr node) {
    TreeNodePtr current = node;
    while (current != NULL) {
        processUnlabeledStatement(current);
        current = current->next;
    }
}
// ...

void processExpression(TreeNodePtr node) {
    if(node->category != EXPRESSION_NODE) {
        UnexpectedNodeCategoryError(EXPRESSION_NODE, node->category);
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
            UnexpectedChildNodeCategoryError(EXPRESSION_NODE, node->category);
    }
}

void processBinaryOpExpression(TreeNodePtr node) {
    if(node->category != BINARY_OPERATOR_EXPRESSION_NODE) {
        UnexpectedNodeCategoryError(BINARY_OPERATOR_EXPRESSION_NODE, node->category);
    }

    TreeNodePtr termNode = node->subtrees[0];
    TreeNodePtr additiveOperationNode = node->subtrees[1];

    processTerm(termNode);
    processAdditiveOperation(additiveOperationNode);
}

void processUnopExpression(TreeNodePtr node) {
    if(node->category != UNARY_OPERATOR_EXPRESSION_NODE) {
        UnexpectedNodeCategoryError(UNARY_OPERATOR_EXPRESSION_NODE, node->category);
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
        UnexpectedNodeCategoryError(ADDITIVE_OPERATION_NODE, node->category);
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
        UnexpectedNodeCategoryError(TERM_NODE, node->category);
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
        UnexpectedNodeCategoryError(MULTIPLICATIVE_OPERATION_NODE, node->category);
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
        UnexpectedNodeCategoryError(FACTOR_NODE, node->category);
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
            UnexpectedChildNodeCategoryError(FACTOR_NODE, specificFactor->category);
    }
}


void processInteger(TreeNodePtr node) {
    if(node->category != INTEGER_NODE) {
        UnexpectedNodeCategoryError(INTEGER_NODE, node->category);
    }

    char* integer = node->name;
    addCommand("LDCT %s", integer);
}


void processRelationalOperator(TreeNodePtr node) {
    if(node->category != RELATIONAL_OPERATOR_NODE) {
        UnexpectedNodeCategoryError(RELATIONAL_OPERATOR_NODE, node->category);
    }

    TreeNodePtr operatorNode = node->subtrees[0];
    if(operatorNode->category == LESS_OR_EQUAL_NODE) {
        addCommand("LEQU");
    } else if(operatorNode->category == LESS_NODE) {
        addCommand("LESS");
    } else if(operatorNode->category == EQUAL_NODE) {
        addCommand("EQUA");
    } else if(operatorNode->category == DIFFERENT_NODE) {
        addCommand("DIFF");
    } else if(operatorNode->category == GREATER_OR_EQUAL_NODE) {
        addCommand("GEQU");
    } else if(operatorNode->category == GREATER_NODE) {
        addCommand("GRTR");
    } else {
        UnexpectedChildNodeCategoryError(RELATIONAL_OPERATOR_NODE, operatorNode->category);
    }
}

void processAdditiveOperator(TreeNodePtr node) {
    if(node->category != ADDITIVE_OPERATOR_NODE) {
        UnexpectedNodeCategoryError(ADDITIVE_OPERATOR_NODE, node->category);
    }

    TreeNodePtr operatorNode = node->subtrees[0];
    if(operatorNode->category == PLUS_NODE) {
        addCommand("ADDD");
    } else if(operatorNode->category == MINUS_NODE) {
        addCommand("SUBT");
    } else if(operatorNode->category == OR_NODE) {
        addCommand("LORR");
    } else {
        UnexpectedChildNodeCategoryError(ADDITIVE_OPERATOR_NODE, operatorNode->category);
    }
}

void processUnaryOperator(TreeNodePtr node) {
    if(node->category != UNARY_OPERATOR_NODE) {
        UnexpectedNodeCategoryError(UNARY_OPERATOR_NODE, node->category);
    }

    TreeNodePtr operatorNode = node->subtrees[0];
    if(operatorNode->category == PLUS_NODE) {
        // this operator has no practical effect
    } else if(operatorNode->category == MINUS_NODE) {
        addCommand("NEGT");
    } else if(operatorNode->category == NOT_NODE) {
        addCommand("LNOT");
    } else {
        UnexpectedChildNodeCategoryError(UNARY_OPERATOR_NODE, operatorNode->category);
    }
}

void processMultiplicativeOperator(TreeNodePtr node) {
    if(node->category != MULTIPLICATIVE_OPERATOR_NODE) {
        UnexpectedNodeCategoryError(MULTIPLICATIVE_OPERATOR_NODE, node->category);
    }

    TreeNodePtr operatorNode = node->subtrees[0];
    if(operatorNode->category == MULTIPLY_NODE) {
        addCommand("MULT");
    } else if(operatorNode->category == DIV_NODE) {
        addCommand("DIVI");
    } else if(operatorNode->category == AND_NODE) {
        addCommand("LAND");
    } else {
        UnexpectedChildNodeCategoryError(MULTIPLICATIVE_OPERATOR_NODE, operatorNode->category);
    }
}

void UnexpectedNodeCategoryError(NodeCategory expected, NodeCategory gotten) {
    char message[50];
    sprintf(message, "Expected %s, but got %s", getCategoryName(expected), getCategoryName(gotten));
    SemanticError(message);
}

void UnexpectedChildNodeCategoryError(NodeCategory fatherNodeCategory, NodeCategory childNodeCategory) {
    char message[100];
    sprintf(message, "For node category %s, got unexpected child node category %s",
            getCategoryName(fatherNodeCategory), getCategoryName(childNodeCategory));
    SemanticError(message);
}

Queue *getProgramQueue() {
    if(programQueue == NULL) {
        programQueue = newQueue();
    }
    return programQueue;
}

void addCommand(const char* commandFormat, ...) {
    Queue* programQueue = getProgramQueue();

    va_list args;
    va_start(args, commandFormat);
    char* command = malloc(sizeof(char)*255);
    vsprintf(command, commandFormat, args);
    va_end(args);

    enqueue(programQueue, command);
}

void printProgram() {
    Queue* programQueue = getProgramQueue();

    while (programQueue->size > 0) {
        char* command = (char*) dequeue(programQueue);
        printf("      %s\n", command);
        free(command);
    }
}
