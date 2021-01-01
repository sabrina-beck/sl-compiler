#include "codegen.h"
#include "tree.h"
#include "slc.h"
#include "utils.h"
#include "datastructures.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

void processFunction(TreeNodePtr node, bool mainFunction);

/* Function Header */
SymbolTableEntryPtr processFunctionHeader(TreeNodePtr node);

TypeDescriptorPtr processFunctionReturnType(TreeNodePtr node);

List* processFormalParameterList(TreeNodePtr node);
List* processFormalParameter(TreeNodePtr node, int* displacement);
List* processParameterByReferenceDeclaration(TreeNodePtr node, int* displacement);
List* processParameterByValueDeclaration(TreeNodePtr node, int* displacement);
List* processParameterDeclaration(TreeNodePtr node, ParameterPassage passage, int* displacement);
SymbolTableEntryPtr processFunctionParameterDeclaration(TreeNodePtr node, int* displacement);

/* Block */
void processBlock(TreeNodePtr node);

void processLabels(TreeNodePtr node);

void processTypes(TreeNodePtr node);
void processTypeDeclaration(TreeNodePtr node);

void processVariables(TreeNodePtr node);
void processVariableDeclaration(TreeNodePtr node, int* displacement);


// ...
Stack* processIdentifiersAsStack(TreeNodePtr node);
Queue* processIdentifiersAsQueue(TreeNodePtr node);
TypeDescriptorPtr processTypeIdentifier(TreeNodePtr node);
char* processIdentifier(TreeNodePtr node);

TypeDescriptorPtr processType(TreeNodePtr node);
int processArraySizeDeclaration(TreeNodePtr node);
// ...

void processUnlabeledStatement(TreeNodePtr node);
void processAssignment(TreeNodePtr node);

// ...
void processConditional(TreeNodePtr node);
void processRepetitive(TreeNodePtr node);

void processCompound(TreeNodePtr node);
void processUnlabeledStatementList(TreeNodePtr node);

// ...

TypeDescriptorPtr processExpression(TreeNodePtr node);
TypeDescriptorPtr routeExpressionSubtree(TreeNodePtr node);
TypeDescriptorPtr processBinaryOpExpression(TreeNodePtr node);
TypeDescriptorPtr processUnopExpression(TreeNodePtr node);
TypeDescriptorPtr processAdditiveOperation(TreeNodePtr node);
TypeDescriptorPtr processTerm(TreeNodePtr node);
TypeDescriptorPtr processMultiplicativeOperation(TreeNodePtr node);
TypeDescriptorPtr processFactor(TreeNodePtr node);
int processInteger(TreeNodePtr node);
TypeDescriptorPtr processRelationalOperator(TreeNodePtr node);
TypeDescriptorPtr processAdditiveOperator(TreeNodePtr node);
TypeDescriptorPtr processUnaryOperator(TreeNodePtr node);
TypeDescriptorPtr processMultiplicativeOperator(TreeNodePtr node);

/** Error Handling **/
void UnexpectedNodeCategoryError(NodeCategory expected, NodeCategory gotten);
void UnexpectedChildNodeCategoryError(NodeCategory fatherNodeCategory, NodeCategory childNodeCategory);

/** Queue with the final program **/
void addCommand(const char* commandFormat, ...);
void printProgram();

/**
 * Symbol Table
 **/
SymbolTablePtr getSymbolTable();

/**
 * Function Level
 **/
int currentFunctionLevel = 0;

/**
 * Implementations
 **/
void processProgram(void *p) {
    TreeNodePtr treeRoot = (TreeNodePtr) p;

    processFunction(treeRoot, true);

    addCommand("END ");

    printProgram();
}

void processFunction(TreeNodePtr node, bool mainFunction) {
    if(node->category != FUNCTION_NODE) {
        UnexpectedNodeCategoryError(FUNCTION_NODE, node->category);
    }

    if(mainFunction) {
        addCommand("MAIN");
    } else {
        addCommand("ENFN %d", currentFunctionLevel);
    }

    TreeNodePtr functionHeaderNode = node->subtrees[0];
    SymbolTableEntryPtr functionSymbolTableEntry = processFunctionHeader(functionHeaderNode);
    addSymbolTableEntry(getSymbolTable(), functionSymbolTableEntry);

    TreeNodePtr blockNode = node->subtrees[1];
    // TODO processBlock()

    if(mainFunction) {
        addCommand("STOP");
    } else {
        // TODO addCommand("RTRN %d", numberOfParameters);
    }
}

/** Function Header **/

SymbolTableEntryPtr processFunctionHeader(TreeNodePtr node) {
    if(node->category != FUNCTION_HEADER_NODE) {
        UnexpectedNodeCategoryError(FUNCTION_HEADER_NODE, node->category);
    }

    TreeNodePtr returnTypeNode = node->subtrees[0];
    TypeDescriptorPtr returnType = processFunctionReturnType(returnTypeNode);

    TreeNodePtr identifierNode = node->subtrees[1];
    char* identifier = processIdentifier(identifierNode);

    TreeNodePtr formalParametersNode = node->subtrees[2];
    List* parameters = processFormalParameterList(formalParametersNode);

    return newFunctionDescriptor(currentFunctionLevel, identifier, returnType, parameters);
}

TypeDescriptorPtr processFunctionReturnType(TreeNodePtr node) {
    if(node == NULL) { // VOID function
        return NULL;
    }

    return processTypeIdentifier(node);
}

List* processFormalParameterList(TreeNodePtr node) {
    int parametersDisplacement = -5; // FIXME explain
    processFormalParameter(node, &parametersDisplacement);
}

List* processFormalParameter(TreeNodePtr node, int* displacement) {
    if(node == NULL) {
        return NULL;
    }

    List* nextParams = processFormalParameter(node->next, displacement); // process parameters in reversed order

    List* params;
    switch (node->category) {
        case EXPRESSION_PARAMETER_BY_REFERENCE_NODE:
            params = processParameterByReferenceDeclaration(node, displacement);
            break;
        case EXPRESSION_PARAMETER_BY_VALUE_NODE:
            params = processParameterByValueDeclaration(node, displacement);
            break;
        case FUNCTION_PARAMETER_NODE:
            params = newList();
            SymbolTableEntryPtr functionParameter = processFunctionParameterDeclaration(node, displacement);
            add(params, functionParameter);
            break;
        default:
            UnexpectedChildNodeCategoryError(FUNCTION_HEADER_NODE, node->category);
    }

    params = concat(params, nextParams);
    free(nextParams);

    return params;
}

List* processParameterByReferenceDeclaration(TreeNodePtr node, int* displacement) {
    if(node->category != EXPRESSION_PARAMETER_BY_REFERENCE_NODE) {
        UnexpectedNodeCategoryError(EXPRESSION_PARAMETER_BY_REFERENCE_NODE, node->category);
    }

    return processParameterDeclaration(node, VARIABLE_PARAMETER, displacement);
}

List* processParameterByValueDeclaration(TreeNodePtr node, int* displacement) {
    if(node->category != EXPRESSION_PARAMETER_BY_VALUE_NODE) {
        UnexpectedNodeCategoryError(EXPRESSION_PARAMETER_BY_VALUE_NODE, node->category);
    }

    return processParameterDeclaration(node, VALUE_PARAMETER, displacement);
}

List* processParameterDeclaration(TreeNodePtr node, ParameterPassage passage, int* displacement) {
    TreeNodePtr parameterIdentifierNode = node->subtrees[0];
    Stack* identifiers = processIdentifiersAsStack(node);

    TreeNodePtr typeIdentifierNode = node->subtrees[1];
    TypeDescriptorPtr type = processTypeIdentifier(typeIdentifierNode);

    List* parameters = newList();

    char* identifier = pop(identifiers);
    while (identifier != NULL) {
        SymbolTableEntryPtr entry = newParameter(currentFunctionLevel, identifier, *displacement, type, passage);
        add(parameters, entry);

        *displacement -= type->size;
    }
    free(identifiers);

    return parameters;
}

SymbolTableEntryPtr processFunctionParameterDeclaration(TreeNodePtr node, int* displacement) {
    if(node->category != FUNCTION_PARAMETER_NODE) {
        UnexpectedNodeCategoryError(FUNCTION_PARAMETER_NODE, node->category);
    }

    TreeNodePtr functionHeaderNode = node->subtrees[0];
    SymbolTableEntryPtr functionEntry = processFunctionHeader(functionHeaderNode);

    SymbolTableEntryPtr functionParameterEntry = newFunctionParameter(functionEntry, *displacement);
    *displacement -= functionParameterEntry->description.parameterDescriptor->type->size;

    return functionParameterEntry;
}

/** Block **/

void processBlock(TreeNodePtr node) {
    if(node->category != BLOCK_NODE) {
        UnexpectedNodeCategoryError(BLOCK_NODE, node->category);
    }

    TreeNodePtr labelsNode = node->subtrees[0];
    processLabels(labelsNode);
    TreeNodePtr typesNode = node->subtrees[1];
    processTypes(typesNode);
    TreeNodePtr variablesNode = node->subtrees[2];
    // TODO
    TreeNodePtr functionsNode = node->subtrees[3];
    // TODO
    TreeNodePtr bodyNode = node->subtrees[4];
    // TODO
}

void processLabels(TreeNodePtr node) {
    if(node == NULL) {
        return;
    }

    if(node->category != LABELS_NODE) {
        UnexpectedNodeCategoryError(LABELS_NODE, node->category);
    }

    TreeNodePtr identifiersNode = node->subtrees[0];
    Queue* identifiers = processIdentifiersAsQueue(identifiersNode);
    while (identifiers->front != NULL) {
        char* identifier = (char*) dequeue(identifiers);
        SymbolTableEntryPtr labelEntry = newLabel(currentFunctionLevel, identifier);
        addSymbolTableEntry(getSymbolTable(), labelEntry);
    }
    free(identifiers);
}

void processTypes(TreeNodePtr node) {
    if(node == NULL) {
        return;
    }

    if(node->category != TYPES_NODE) {
        UnexpectedNodeCategoryError(TYPES_NODE, node->category);
    }

    TreeNodePtr typeDeclarationNode = node->subtrees[0];
    while (typeDeclarationNode != NULL) {
        processTypeDeclaration(typeDeclarationNode);
        typeDeclarationNode = typeDeclarationNode->next;
    }
}

void processTypeDeclaration(TreeNodePtr node) {
    if(node->category != TYPE_DECLARATION_NODE) {
        UnexpectedNodeCategoryError(TYPE_DECLARATION_NODE, node->category);
    }

    TreeNodePtr identifierNode = node->subtrees[0];
    char* identifier = processIdentifier(identifierNode);

    TreeNodePtr typeNode = node->subtrees[1];
    TypeDescriptorPtr type = processType(typeNode);

    SymbolTableEntryPtr typeEntry = newType(currentFunctionLevel, identifier, type);
    addSymbolTableEntry(getSymbolTable(), typeEntry);
}

void processVariables(TreeNodePtr node) {
    if(node == NULL) {
        return;
    }

    if(node->category != VARIABLES_NODE) {
        UnexpectedNodeCategoryError(VARIABLES_NODE, node->category);
    }

    TreeNodePtr variableDeclarationNode = node->subtrees[0];
    int displacement = 0;
    while (variableDeclarationNode != NULL) {
        processVariableDeclaration(variableDeclarationNode, &displacement);
        variableDeclarationNode = variableDeclarationNode->next;
    }
}

void processVariableDeclaration(TreeNodePtr node, int* displacement) {
    if(node->category != DECLARATION_NODE) {
        UnexpectedNodeCategoryError(DECLARATION_NODE, node->category);
    }

    TreeNodePtr identifiersNode = node->subtrees[0];
    Queue* identifiers = processIdentifiersAsQueue(identifiersNode);

    TreeNodePtr typeNode = node->subtrees[1];
    TypeDescriptorPtr type = processType(typeNode);

    while (identifiers->front != NULL) {
        char* identifier = dequeue(identifiers);
        SymbolTableEntryPtr variableEntry = newVariable(currentFunctionLevel, identifier, *displacement, type);
        addSymbolTableEntry(getSymbolTable(), variableEntry);
        *displacement += type->size;
    }

    free(identifiers);
}

// ...

Stack* processIdentifiersAsStack(TreeNodePtr node) {
    Stack * identifiers = newStack();

    TreeNodePtr current = node;
    while (current != NULL) {
        char* identifier = processIdentifier(current);
        push(identifiers, identifier);
        current = current->next;
    }

    return identifiers;
}

Queue* processIdentifiersAsQueue(TreeNodePtr node) {
    Queue* identifiers = newQueue();

    TreeNodePtr current = node;
    while (current != NULL) {
        char* identifier = processIdentifier(current);
        enqueue(identifiers, identifier);
        current = current->next;
    }

    return identifiers;
}

TypeDescriptorPtr processTypeIdentifier(TreeNodePtr node) {
    SymbolTablePtr symbolTable = getSymbolTable();

    char* identifier = processIdentifier(node);
    SymbolTableEntryPtr entry = findIdentifier(symbolTable, identifier);

    if(entry->category != TYPE_SYMBOL) {
        SemanticError("Expected identifier to be a type but it wasn't");
    }
    return entry->description.typeDescriptor;
}

char* processIdentifier(TreeNodePtr node) {
    if(node->category != IDENTIFIER_NODE) {
        UnexpectedNodeCategoryError(IDENTIFIER_NODE, node->category);
    }
    return node->name;
}

TypeDescriptorPtr processType(TreeNodePtr node) {
    if(node->category != TYPE_NODE) {
        UnexpectedNodeCategoryError(TYPE_NODE, node->category);
    }

    TreeNodePtr identifierNode = node->subtrees[0];
    char* identifier = processIdentifier(identifierNode);

    TreeNodePtr arraySizeDeclarationNode = node->subtrees[1];
    int arrayDimension = 0;
    while (arraySizeDeclarationNode != NULL) {
        arrayDimension += processArraySizeDeclaration(arraySizeDeclarationNode);
        arraySizeDeclarationNode = arraySizeDeclarationNode->next;
    }
}

int processArraySizeDeclaration(TreeNodePtr node) {
    if(node == NULL) {
        return 0;
    }

    if(node->category != ARRAY_SIZE_NODE) {
        UnexpectedNodeCategoryError(ARRAY_SIZE_NODE, node->category);
    }

    TreeNodePtr integerNode = node->subtrees[0];
    return processInteger(integerNode);
}

// ...

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

TypeDescriptorPtr processExpression(TreeNodePtr node) {
    if(node->category != EXPRESSION_NODE) {
        UnexpectedNodeCategoryError(EXPRESSION_NODE, node->category);
    }

    TreeNodePtr childExprNode = node->subtrees[0];
    TreeNodePtr relationalOperatorNode = node->subtrees[1];
    TreeNodePtr binaryopExprNode = node->subtrees[2];

    TypeDescriptorPtr firstExprType = routeExpressionSubtree(childExprNode);
    TypeDescriptorPtr secondExprType = processBinaryOpExpression(binaryopExprNode);
    TypeDescriptorPtr operatorType = processRelationalOperator(relationalOperatorNode);

    if(!equivalentTypes(firstExprType, secondExprType)) {
        SemanticError("Expressions of incompatible type");
    }

    if(operatorType != NULL) {
        return operatorType;
    }
    return firstExprType;
}

TypeDescriptorPtr routeExpressionSubtree(TreeNodePtr node) {
    switch (node->category) {
        case BINARY_OPERATOR_EXPRESSION_NODE:
            return processBinaryOpExpression(node);
        case UNARY_OPERATOR_EXPRESSION_NODE:
            return processUnopExpression(node);
        default:
            UnexpectedChildNodeCategoryError(EXPRESSION_NODE, node->category);
    }
}

TypeDescriptorPtr processBinaryOpExpression(TreeNodePtr node) {
    if(node->category != BINARY_OPERATOR_EXPRESSION_NODE) {
        UnexpectedNodeCategoryError(BINARY_OPERATOR_EXPRESSION_NODE, node->category);
    }

    TreeNodePtr termNode = node->subtrees[0];
    TreeNodePtr additiveOperationNode = node->subtrees[1];

    TypeDescriptorPtr termType = processTerm(termNode);
    TypeDescriptorPtr additiveOpType = processAdditiveOperation(additiveOperationNode);

    if(!equivalentTypes(termType, additiveOpType)) {
        SemanticError("Expression's terms have incompatible types");
    }

    return termType;
}

TypeDescriptorPtr processUnopExpression(TreeNodePtr node) {
    if(node->category != UNARY_OPERATOR_EXPRESSION_NODE) {
        UnexpectedNodeCategoryError(UNARY_OPERATOR_EXPRESSION_NODE, node->category);
    }

    TreeNodePtr unaryOperatorNode = node->subtrees[0];
    TreeNodePtr termNode = node->subtrees[1];
    TreeNodePtr additiveOperationNode = node->subtrees[2];

    TypeDescriptorPtr termType = processTerm(termNode);
    TypeDescriptorPtr operatorType = processUnaryOperator(unaryOperatorNode);
    TypeDescriptorPtr additiveOpType = processAdditiveOperation(additiveOperationNode);

    if(!equivalentTypes(termType, operatorType)) {
        SemanticError("Expression's term type incompatible with unary operator");
    }

    if(!equivalentTypes(termType, additiveOpType)) {
        SemanticError("Expression's terms have incompatible types");
    }

    return operatorType;
}

TypeDescriptorPtr processAdditiveOperation(TreeNodePtr node) {
    if(node == NULL) {
        return NULL;
    }

    if(node->category != ADDITIVE_OPERATION_NODE) {
        UnexpectedNodeCategoryError(ADDITIVE_OPERATION_NODE, node->category);
    }

    TreeNodePtr operatorNode = node->subtrees[0];
    TreeNodePtr termNode = node->subtrees[1];
    TreeNodePtr additiveOperationNode = node->subtrees[2];

    TypeDescriptorPtr termType = processTerm(termNode);
    TypeDescriptorPtr operatorType = processAdditiveOperator(operatorNode);
    TypeDescriptorPtr additiveOpType = processAdditiveOperation(additiveOperationNode);

    if(!equivalentTypes(termType, additiveOpType)) {
        SemanticError("Expression's terms have incompatible types");
    }

    if(!equivalentTypes(termType, operatorType)) {
        SemanticError("Expression's terms type incompatible with operator");
    }

    return operatorType;
}

TypeDescriptorPtr processTerm(TreeNodePtr node) {
    if(node->category != TERM_NODE) {
        UnexpectedNodeCategoryError(TERM_NODE, node->category);
    }

    TreeNodePtr factorNode = node->subtrees[0];
    TreeNodePtr multiplicativeOperationNode = node->subtrees[1];

    TypeDescriptorPtr factorType = processFactor(factorNode);
    TypeDescriptorPtr multiplicativeOpType = processMultiplicativeOperation(multiplicativeOperationNode);

    if(!equivalentTypes(factorType, multiplicativeOpType)) {
        SemanticError("Term's factors of incompatible types");
    }

    return factorType;
}

TypeDescriptorPtr processMultiplicativeOperation(TreeNodePtr node) {
    if(node == NULL) {
        return NULL;
    }

    if(node->category != MULTIPLICATIVE_OPERATION_NODE) {
        UnexpectedNodeCategoryError(MULTIPLICATIVE_OPERATION_NODE, node->category);
    }

    TreeNodePtr operatorNode = node->subtrees[0];
    TreeNodePtr factorNode = node->subtrees[1];
    TreeNodePtr multiplicativeOperationNode = node->subtrees[2];

    TypeDescriptorPtr factorType = processFactor(factorNode);
    TypeDescriptorPtr operatorType = processMultiplicativeOperator(operatorNode);
    TypeDescriptorPtr multiplicativeOpType = processMultiplicativeOperation(multiplicativeOperationNode);

    if(!equivalentTypes(factorType, multiplicativeOpType)) {
        SemanticError("Term's factors have incompatible types");
    }

    if(!equivalentTypes(factorType, operatorType)) {
        SemanticError("Term's factors type incompatible with operator");
    }

    return operatorType;
}

TypeDescriptorPtr processFactor(TreeNodePtr node) {
    if(node->category != FACTOR_NODE) {
        UnexpectedNodeCategoryError(FACTOR_NODE, node->category);
    }

    TreeNodePtr specificFactor = node->subtrees[0];
    switch (specificFactor->category) {
        case VARIABLE_NODE:
            //TODO return processVariable();
            //TODO LDVL
            break;
        case INTEGER_NODE: {
            int integer = processInteger(specificFactor);
            addCommand("LDCT %d", integer);
            return getSymbolTable()->integerTypeDescriptor;
        }
        case FUNCTION_CALL_NODE:
            //TODO return processFunctionCall();
            break;
        case EXPRESSION_NODE:
            return processExpression(node);
        default:
            UnexpectedChildNodeCategoryError(FACTOR_NODE, specificFactor->category);
    }
}


int processInteger(TreeNodePtr node) {
    if(node->category != INTEGER_NODE) {
        UnexpectedNodeCategoryError(INTEGER_NODE, node->category);
    }

    char* integerStr = node->name;

    char* end;
    int integer = strtol(integerStr, &end, 10);
    if(end != NULL && *end != '\0') {
        SemanticError("Integer that it isn't an integer");
    }

    return integer;
}


TypeDescriptorPtr processRelationalOperator(TreeNodePtr node) {
    if(node->category != RELATIONAL_OPERATOR_NODE) {
        UnexpectedNodeCategoryError(RELATIONAL_OPERATOR_NODE, node->category);
    }

    TreeNodePtr operatorNode = node->subtrees[0];
    switch (operatorNode->category) {
        case LESS_OR_EQUAL_NODE:
            addCommand("LEQU");
            break;
        case LESS_NODE:
            addCommand("LESS");
            break;
        case EQUAL_NODE:
            addCommand("EQUA");
            break;
        case DIFFERENT_NODE:
            addCommand("DIFF");
            break;
        case GREATER_OR_EQUAL_NODE:
            addCommand("GEQU");
            break;
        case GREATER_NODE:
            addCommand("GRTR");
            break;
        default:
            UnexpectedChildNodeCategoryError(RELATIONAL_OPERATOR_NODE, operatorNode->category);
    }

    return getSymbolTable()->booleanTypeDescriptor;
}

TypeDescriptorPtr processAdditiveOperator(TreeNodePtr node) {
    if(node->category != ADDITIVE_OPERATOR_NODE) {
        UnexpectedNodeCategoryError(ADDITIVE_OPERATOR_NODE, node->category);
    }

    TreeNodePtr operatorNode = node->subtrees[0];
    switch (operatorNode->category) {
        case PLUS_NODE:
            addCommand("ADDD");
            return getSymbolTable()->integerTypeDescriptor;
        case MINUS_NODE:
            addCommand("SUBT");
            return getSymbolTable()->integerTypeDescriptor;
        case OR_NODE:
            addCommand("LORR");
            return getSymbolTable()->booleanTypeDescriptor;
        default:
            UnexpectedChildNodeCategoryError(ADDITIVE_OPERATOR_NODE, operatorNode->category);
    }
}

TypeDescriptorPtr processUnaryOperator(TreeNodePtr node) {
    if(node->category != UNARY_OPERATOR_NODE) {
        UnexpectedNodeCategoryError(UNARY_OPERATOR_NODE, node->category);
    }

    TreeNodePtr operatorNode = node->subtrees[0];
    switch (operatorNode->category) {
        case PLUS_NODE:
            // this operator has no practical effect
            return getSymbolTable()->integerTypeDescriptor;
        case MINUS_NODE:
            addCommand("NEGT");
            return getSymbolTable()->integerTypeDescriptor;
        case NOT_NODE:
            addCommand("LNOT");
            return getSymbolTable()->booleanTypeDescriptor;
        default:
            UnexpectedChildNodeCategoryError(UNARY_OPERATOR_NODE, operatorNode->category);
    }
}

TypeDescriptorPtr processMultiplicativeOperator(TreeNodePtr node) {
    if(node->category != MULTIPLICATIVE_OPERATOR_NODE) {
        UnexpectedNodeCategoryError(MULTIPLICATIVE_OPERATOR_NODE, node->category);
    }

    TreeNodePtr operatorNode = node->subtrees[0];
    switch(operatorNode->category) {
        case MULTIPLY_NODE:
            addCommand("MULT");
            return getSymbolTable()->integerTypeDescriptor;
        case DIV_NODE:
            addCommand("DIVI");
            return getSymbolTable()->integerTypeDescriptor;
        case AND_NODE:
            addCommand("LAND");
            return getSymbolTable()->booleanTypeDescriptor;
        default:
            UnexpectedChildNodeCategoryError(MULTIPLICATIVE_OPERATOR_NODE, operatorNode->category);
    }
}

/** Semantic Errors handling functions **/

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

/** Program String managament functions **/
Queue *programQueue = NULL;
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

/**
 * Symbol Table
 **/
SymbolTablePtr symbolTable = NULL;
SymbolTablePtr getSymbolTable() {
    if(symbolTable == NULL) {
        symbolTable = initializeSymbolTable();
    }
    return symbolTable;
}