#include "codegen.h"
#include "tree.h"
#include "slc.h"
#include "utils.h"
#include "datastructures.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

typedef struct {
    TypeDescriptorPtr type;
    bool address;
    bool array;
    int level;
    int displacement;
} Variable, *VariablePtr;

void processFunction(TreeNodePtr node, bool mainFunction);

/* Function Header */
SymbolTableEntryPtr processFunctionHeader(TreeNodePtr node, char* mepaLabel);

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

int processVariables(TreeNodePtr node);
void processVariableDeclaration(TreeNodePtr node, int* displacement);

void processFunctions(TreeNodePtr node);

/* Identifier & Type */
Stack* processIdentifiersAsStack(TreeNodePtr node);
Queue* processIdentifiersAsQueue(TreeNodePtr node);
TypeDescriptorPtr processTypeIdentifier(TreeNodePtr node);
char* processIdentifier(TreeNodePtr node);

TypeDescriptorPtr processType(TreeNodePtr node);
TypeDescriptorPtr processArraySizeDeclaration(TreeNodePtr node, TypeDescriptorPtr elementType);

/* Body */
void processBody(TreeNodePtr node, int allocatedSizeForVariables);

void processStatement(TreeNodePtr node, int allocatedSizeForVariables);

void processLabel(TreeNodePtr node, int allocatedSizeForVariables);

void processUnlabeledStatement(TreeNodePtr node);

void processAssignment(TreeNodePtr node);
VariablePtr processVariable(TreeNodePtr node);
TypeDescriptorPtr processArrayIndexList(TreeNodePtr node, bool address, int level, int displacement, TypeDescriptorPtr variableTypeDescriptor);
TypeDescriptorPtr processArrayIndex(TreeNodePtr node, TypeDescriptorPtr arrayTypeDescriptor);

TypeDescriptorPtr processFunctionCall(TreeNodePtr node);
void processExpressionList(TreeNodePtr node, List* expectedParams);
TreeNodePtr getVariableExpression(TreeNodePtr node);

void processGoto(TreeNodePtr node);

void processReturn(TreeNodePtr node);

void processConditional(TreeNodePtr node);
void processRepetitive(TreeNodePtr node);

void processCompound(TreeNodePtr node);
void processUnlabeledStatementList(TreeNodePtr node);

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
int currentFunctionLevel = -1;

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

    currentFunctionLevel++;

    char* mepaLabel = nextMEPALabel();
    if(mainFunction) {
        addCommand("MAIN");
    } else {
        addCommand("%s: ENFN %d", mepaLabel, currentFunctionLevel);
    }

    TreeNodePtr functionHeaderNode = node->subtrees[0];
    SymbolTableEntryPtr functionSymbolTableEntry = processFunctionHeader(functionHeaderNode, mepaLabel);
    if(!mainFunction) {
        addSymbolTableEntry(getSymbolTable(), functionSymbolTableEntry);
    } else {
        free(functionSymbolTableEntry); // TODO create cascade free on utils for any kind
    }

    TreeNodePtr blockNode = node->subtrees[1];
    processBlock(blockNode);

    if(mainFunction) {
        addCommand("STOP");
    } else {
        addCommand("RTRN %d", parametersTotalSize(functionSymbolTableEntry));
    }

    currentFunctionLevel--;
    changeToLevelScope(getSymbolTable(), currentFunctionLevel);
}

/** Function Header **/

SymbolTableEntryPtr processFunctionHeader(TreeNodePtr node, char* mepaLabel) {
    if(node->category != FUNCTION_HEADER_NODE) {
        UnexpectedNodeCategoryError(FUNCTION_HEADER_NODE, node->category);
    }

    TreeNodePtr returnTypeNode = node->subtrees[0];
    TypeDescriptorPtr returnType = processFunctionReturnType(returnTypeNode);

    TreeNodePtr identifierNode = node->subtrees[1];
    char* identifier = processIdentifier(identifierNode);

    TreeNodePtr formalParametersNode = node->subtrees[2];
    List* parameters = processFormalParameterList(formalParametersNode);

    return newFunctionDescriptor(currentFunctionLevel, identifier, mepaLabel, returnType, parameters);
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
    SymbolTableEntryPtr functionEntry = processFunctionHeader(functionHeaderNode, NULL);

    SymbolTableEntryPtr functionParameterEntry = newFunctionParameter(functionEntry, *displacement);
    *displacement -= functionParameterEntry->description.parameterDescriptor->type->size;

    free(functionEntry->description.functionDescriptor);
    free(functionEntry);
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
    int allocatedSizeForVariables = processVariables(variablesNode);
    if(allocatedSizeForVariables > 0) {
        addCommand("ALOC %d", allocatedSizeForVariables);
    }

    TreeNodePtr functionsNode = node->subtrees[3];
    processFunctions(functionsNode);

    TreeNodePtr bodyNode = node->subtrees[4];
    processBody(bodyNode, allocatedSizeForVariables);

    if(allocatedSizeForVariables > 0) {
        addCommand("DLOC %d", allocatedSizeForVariables);
    }
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

int processVariables(TreeNodePtr node) {
    if(node == NULL) {
        return 0;
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

    return displacement;
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

void processFunctions(TreeNodePtr node) {
    if(node == NULL) {
        return;
    }

    if(node->category != FUNCTIONS_NODE) {
        UnexpectedNodeCategoryError(FUNCTIONS_NODE, node->category);
    }

    TreeNodePtr functionNode = node->subtrees[0];
    while (functionNode != NULL) {
        processFunction(functionNode, false);
        functionNode = functionNode->next;
    }
}

/* Identifier & Type */

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
    SymbolTableEntryPtr entry = findIdentifier(getSymbolTable(), identifier);

    if(entry->category != TYPE_SYMBOL) {
        SemanticError("Expected type identifier");
    }

    TypeDescriptorPtr elementType = entry->description.typeDescriptor;

    TreeNodePtr arraySizeNodes = node->subtrees[1];
    TypeDescriptorPtr arrayType = processArraySizeDeclaration(arraySizeNodes, elementType);

    if(arrayType != NULL) {
        return arrayType;
    }

    return elementType;
}

TypeDescriptorPtr processArraySizeDeclaration(TreeNodePtr node, TypeDescriptorPtr elementType) {
    if(node == NULL) {
        return 0;
    }

    if(node->category != ARRAY_SIZE_NODE) {
        UnexpectedNodeCategoryError(ARRAY_SIZE_NODE, node->category);
    }

    TypeDescriptorPtr subArrayType = processArraySizeDeclaration(node->next, elementType);

    TreeNodePtr integerNode = node->subtrees[0];
    int dimension = processInteger(integerNode);

    TypeDescriptorPtr currentElementType = subArrayType;
    if(subArrayType == NULL) {
        currentElementType = elementType;
    }

    int size = dimension * currentElementType->size;

    return newArrayType(size, dimension, currentElementType);
}

/* Body */

void processBody(TreeNodePtr node, int allocatedSizeForVariables) {
    if(node->category != BODY_NODE) {
        UnexpectedNodeCategoryError(BODY_NODE, node->category);
    }

    TreeNodePtr statementNode = node->subtrees[0];
    while (statementNode != NULL) {
        processStatement(statementNode, allocatedSizeForVariables);
        statementNode = statementNode->next;
    }
}

void processStatement(TreeNodePtr node, int allocatedSizeForVariables) {
    if(node->category != STATEMENT_NODE) {
        UnexpectedNodeCategoryError(STATEMENT_NODE, node->category);
    }

    TreeNodePtr firstChild = node->subtrees[0];
    if(firstChild->category != LABEL_NODE) {
        processUnlabeledStatement(firstChild);
        return;
    }

    processLabel(firstChild, allocatedSizeForVariables);

    TreeNodePtr unlabeledStatamentNode = node->subtrees[1];
    processUnlabeledStatement(unlabeledStatamentNode);

}

void processLabel(TreeNodePtr node, int allocatedSizeForVariables) {
    if(node == NULL) {
        return;
    }

    if(node->category != LABEL_NODE) {
        UnexpectedNodeCategoryError(LABEL_NODE, node->category);
    }

    TreeNodePtr identifierNode = node->subtrees[0];
    char* identifier = processIdentifier(identifierNode);

    SymbolTableEntryPtr symbolTableEntry = findIdentifier(getSymbolTable(), identifier);
    if(symbolTableEntry == NULL) {
        SemanticError("Label not declared");
    }

    if(symbolTableEntry->category != LABEL_SYMBOL) {
        SemanticError("Identifier used as label, but it isn't a label");
    }

    if(symbolTableEntry->description.labelDescriptor->defined) {
        SemanticError("Label already used");
    }

    LabelDescriptorPtr labelDescriptor = symbolTableEntry->description.labelDescriptor;
    labelDescriptor->defined = true;

    // since there are only statements at this level, the current activation record displacement on the stack
    // will always be equal to the size of its alocated variables
    addCommand("ENLB %s, %d", labelDescriptor->mepaLabel, allocatedSizeForVariables);
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
            processFunctionCall(node);
        break;
        case GOTO_NODE:
            processGoto(node);
        break;
        case RETURN_NODE:
            processReturn(node);
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

    VariablePtr variable = processVariable(variableNode);
    TypeDescriptorPtr exprType = processExpression(expressionNode);

    if(!equivalentTypes(exprType, variable->type)) {
        SemanticError("Trying to assign value to variable of incompatible type");
    }

    if(variable->array) {
        addCommand("STMV %d", variable->type->size);
    } else {
        if(variable->address) {
            addCommand("STVI %d, %d", variable->level, variable->displacement);
        } else {
            addCommand("STVL %d, %d", variable->level, variable->displacement);
        }
    }
}

// if it is an indexed array, the position address will be on top of the stack (exec time)
VariablePtr processVariable(TreeNodePtr node) {
    if(node->category != VARIABLE_NODE) {
        UnexpectedNodeCategoryError(VARIABLE_NODE, node->category);
    }

    TreeNodePtr identifierNode = node->subtrees[0];
    char* identifier = processIdentifier(identifierNode);
    SymbolTableEntryPtr entry = findIdentifier(getSymbolTable(), identifier);

    bool address;
    int displacement;
    TypeDescriptorPtr variableDeclarationType;
    switch (entry->category) {
        case VARIABLE_SYMBOL: {
            address = false;
            displacement = entry->description.variableDescriptor->displacement;
            variableDeclarationType = entry->description.variableDescriptor->type;
        }
            break;
        case PARAMETER_SYMBOL: {
            ParameterPassage passage = entry->description.parameterDescriptor->parameterPassage;
            switch (passage) {
                case VALUE_PARAMETER:
                    address = false;
                    break;
                case VARIABLE_PARAMETER:
                    address = true;
                    break;
                default:
                    SemanticError("Unexpected parameter passage type");
            }
            displacement = entry->description.parameterDescriptor->displacement;
            variableDeclarationType = entry->description.parameterDescriptor->type;
        }
            break;
        default:
            SemanticError("Expected variable or parameter as variable"); // FIXME ambiguity
    }

    TreeNodePtr arrayIndexNode = node->subtrees[1];
    TypeDescriptorPtr arrayPositionType = NULL;
    if(entry->category == ARRAY_TYPE) {
        arrayPositionType = processArrayIndexList(arrayIndexNode, address, entry->level, displacement, variableDeclarationType);
    }
    if(arrayIndexNode != NULL && variableDeclarationType->category != ARRAY_TYPE) {
        SemanticError("Trying to index non array variable");
    }

    VariablePtr variable = malloc(sizeof(VariablePtr));

    if(arrayPositionType != NULL) {
        variable->type = arrayPositionType;
        variable->array = true;
    } else {
        variable->type = variableDeclarationType;
        variable->array = false;
    }

    variable->address = address;
    variable->level = entry->level;
    variable->address = displacement;

    return variable;
}

TypeDescriptorPtr processArrayIndexList(TreeNodePtr node, bool address, int level, int displacement, TypeDescriptorPtr variableTypeDescriptor) {
    if(address) {
        addCommand("LDVL %d, %d", level, displacement);
    } else {
        addCommand("LADR %d, %d", level, displacement);
    }

    TreeNodePtr currentIndexNode = node;
    TypeDescriptorPtr currentType = variableTypeDescriptor;
    while (node != NULL) {
        currentType = processArrayIndex(currentIndexNode, currentType);
        currentIndexNode = currentIndexNode->next;
    }

    return currentType;
}

TypeDescriptorPtr processArrayIndex(TreeNodePtr node, TypeDescriptorPtr arrayTypeDescriptor) {
    if(node->category != ARRAY_INDEX_NODE) {
        UnexpectedNodeCategoryError(ARRAY_INDEX_NODE, node->category);
    }

    if(arrayTypeDescriptor->category != ARRAY_TYPE) {
        SemanticError("Expected array type to process array index");
    }

    TreeNodePtr expressionNode = node->subtrees[0];
    TypeDescriptorPtr exprType = processExpression(expressionNode);
    if(!equivalentTypes(exprType, getSymbolTable()->integerTypeDescriptor)) {
        SemanticError("Index should be an integer");
    }
    addCommand("INDX %d", arrayTypeDescriptor->size);

    return arrayTypeDescriptor->description.arrayDescriptor->elementType;
}

TypeDescriptorPtr processFunctionCall(TreeNodePtr node) {
    if(node->category != FUNCTION_CALL_NODE) {
        UnexpectedNodeCategoryError(FUNCTION_CALL_NODE, node->category);
    }

    TreeNodePtr identifierNode = node->subtrees[0];
    char* identifier = processIdentifier(identifierNode);
    SymbolTableEntryPtr functionEntry = findIdentifier(getSymbolTable(), identifier);

    TreeNodePtr expressionListNode = node->subtrees[0];

    switch (functionEntry->category) {
        case PARAMETER_SYMBOL: {
            ParameterDescriptorPtr parameterDescriptor = functionEntry->description.parameterDescriptor;
            if (parameterDescriptor->type->category != FUNCTION_TYPE) {
                SemanticError("Expected function as parameter");
            }
            List* expectedParameters = parameterDescriptor->type->description.functionTypeDescriptor->params;
            processExpressionList(expressionListNode, expectedParameters);
            addCommand("CPFN %d, %d, %d", functionEntry->level, parameterDescriptor->displacement, currentFunctionLevel);

            return parameterDescriptor->type->description.functionTypeDescriptor->returnType;
        }
            break;
        case FUNCTION_SYMBOL: {
            FunctionDescriptorPtr functionDescriptor = functionEntry->description.functionDescriptor;

            processExpressionList(expressionListNode, functionDescriptor->params);
            addCommand("CFUN %s, %d", functionDescriptor->mepaLabel, currentFunctionLevel);

            return functionDescriptor->returnType;
        }
            break;
        default:
            SemanticError("Expected function to perform function call");
    }
}

void processExpressionList(TreeNodePtr node, List* expectedParams) {
    TreeNodePtr currentNode = node;
    LinkedNode* paramNode = expectedParams->front;
    while (paramNode != NULL) {

        SymbolTableEntryPtr parameterEntry = (SymbolTableEntryPtr) paramNode->data;
        ParameterDescriptorPtr expectedParameterDescriptor = parameterEntry->description.parameterDescriptor;

        switch (expectedParameterDescriptor->parameterPassage) {
            case VALUE_PARAMETER: {
                TypeDescriptorPtr expressionType = processExpression(currentNode);

                if(!equivalentTypes(expectedParameterDescriptor->type, expressionType)) {
                    SemanticError("Wrong parameter type on function");
                }
            }
                break;
            case VARIABLE_PARAMETER: {
                TreeNodePtr variableNode = getVariableExpression(currentNode);
                VariablePtr variablePtr = processVariable(variableNode);

                if(!equivalentTypes(expectedParameterDescriptor->type, variablePtr->type)) {
                    SemanticError("Wrong parameter type on function");
                }

                if(variablePtr->array) {
                    // process variable already left the address on top of the stack
                } else {
                    if(variablePtr->address) {
                        addCommand("LDVL %d, %d", variablePtr->level, variablePtr->displacement);
                    } else {
                        addCommand("LADR %d, %d", variablePtr->level, variablePtr->displacement);
                    }
                }
            }
                break;
            case FUNCTION_PARAMETER: {
                TreeNodePtr variableNode = getVariableExpression(currentNode);

                TreeNodePtr identifierNode = variableNode->subtrees[0];
                char* identifier = processIdentifier(identifierNode);

                SymbolTableEntryPtr variableEntry = findIdentifier(getSymbolTable(), identifier);
                switch (variableEntry->category) {
                    case FUNCTION_SYMBOL: {
                        FunctionDescriptorPtr functionDescriptor = variableEntry->description.functionDescriptor;
                        if(!equivalentFunctions(expectedParameterDescriptor->type, functionDescriptor)) {
                            SemanticError("Wrong parameter type on function");
                        }
                        addCommand("LGAD %s, %d", functionDescriptor->mepaLabel, variableEntry->level - 1);
                    }
                        break;
                    case PARAMETER_SYMBOL: {
                        ParameterDescriptorPtr parameterDescriptor = variableEntry->description.parameterDescriptor;
                        TypeDescriptorPtr parameterType = parameterDescriptor->type;
                        if (parameterType->category != FUNCTION_TYPE) {
                            SemanticError("Expected function as parameter");
                        }

                        if(!equivalentTypes(expectedParameterDescriptor->type, parameterDescriptor->type)) {
                            SemanticError("Wrong parameter type on function");
                        }

                        // generalized address is already on the stack
                        // load the MEPA address
                        addCommand("LDVL %s, %d", variableEntry->level, parameterDescriptor->displacement);
                        // load the value of the base register D[k]
                        addCommand("LDVL %s, %d", variableEntry->level, parameterDescriptor->displacement + 1);
                        // load the level k
                        addCommand("LDVL %s, %d", variableEntry->level, parameterDescriptor->displacement + 2);
                    }
                        break;
                    default:
                        SemanticError("Expected function as parameter");
                }

                TreeNodePtr arrayIndexNode = variableNode->subtrees[1];
                if(arrayIndexNode != NULL) {
                    SemanticError("Trying to index function identifier");
                }

            }
                break;
        }

        paramNode = paramNode->next;
        currentNode = currentNode->next;
    }

    if(paramNode->next != NULL) {
        SemanticError("Missing parameters for function call");
    }

    if(currentNode->next != NULL) {
        SemanticError("Too many parameters for function call");
    }
}

/* If the expression is not a single factor, then it is a semantic error */
TreeNodePtr getVariableExpression(TreeNodePtr node) {
    TreeNodePtr binaryOpExpressionNode = node->subtrees[0];
    if(binaryOpExpressionNode->category != BINARY_OPERATOR_EXPRESSION_NODE) {
        UnexpectedNodeCategoryError(BINARY_OPERATOR_EXPRESSION_NODE, binaryOpExpressionNode->category);
    }

    TreeNodePtr relationalOperatorNode = node->subtrees[1];
    TreeNodePtr anotherBinaryIoExpression = node->subtrees[2];
    if(relationalOperatorNode != NULL || anotherBinaryIoExpression != NULL) {
        SemanticError("Expected expression to be single factor, but it is relational expression");
    }

    TreeNodePtr termNode = binaryOpExpressionNode->subtrees[0];
    if(termNode->category != TERM_NODE) {
        UnexpectedNodeCategoryError(TERM_NODE, termNode->category);
    }
    TreeNodePtr additiveOperationNode = binaryOpExpressionNode->subtrees[1];
    if(additiveOperationNode != NULL) {
        SemanticError("Expected expression to be single factor, but it is an additive operation");
    }

    TreeNodePtr factorNode = termNode->subtrees[0];
    if(factorNode->category != FACTOR_NODE) {
        UnexpectedNodeCategoryError(FACTOR_NODE, factorNode->category);
    }
    TreeNodePtr multiplicativeOperationNode = termNode->subtrees[1];
    if(multiplicativeOperationNode != NULL) {
        SemanticError("Expected expression to be single factor, but it is an multiplicative operation");
    }

    TreeNodePtr variableNode = factorNode->subtrees[0];

    if(variableNode->category == EXPRESSION_NODE) {
        variableNode = getVariableExpression(variableNode);
    }

    if(variableNode->category != VARIABLE_NODE) {
        UnexpectedNodeCategoryError(VARIABLE_NODE, variableNode->category);
    }

    return variableNode;
}

void processGoto(TreeNodePtr node) {
    if(node->category != GOTO_NODE) {
        UnexpectedNodeCategoryError(GOTO_NODE, node->category);
    }

    TreeNodePtr identifierNode = node->subtrees[0];
    char* identifier = processIdentifier(identifierNode);

    SymbolTableEntryPtr labelEntry = findIdentifier(getSymbolTable(), identifier);
    if(labelEntry->category != LABEL_SYMBOL) {
        SemanticError("Expected label identifier");
    }

    LabelDescriptorPtr labelDescriptor = labelEntry->description.labelDescriptor;
    if(!labelDescriptor->defined) {
        SemanticError("Use of undefined label");
    }

    addCommand("JUMP %s", labelDescriptor->mepaLabel);
}

void processReturn(TreeNodePtr node) {
    if(node->category != RETURN_NODE) {
        UnexpectedNodeCategoryError(RETURN_NODE, node->category);
    }

    FunctionDescriptorPtr functionDescriptor = findCurrentFunctionDescriptor(getSymbolTable(), currentFunctionLevel);
    if(functionDescriptor == NULL) {
        SemanticError("Unexpected error, can't find current function descriptor");
    }

    TreeNodePtr expressionNode = node->subtrees[0];
    if(expressionNode != NULL) {
        if(functionDescriptor->returnType == NULL) {
            SemanticError("Can't return value for void function");
        }

        TypeDescriptorPtr expressionType = processExpression(expressionNode);
        if (!equivalentTypes(expressionType, functionDescriptor->returnType)) {
            SemanticError("Expression with type different than function return type");
        }

        if(expressionType->size == 1) {
            addCommand("STVL %d, %d", currentFunctionLevel, functionDescriptor->returnDisplacement);
        } else {
            SemanticError("Can't return multiple values"); // TODO check if this is correct
        }
    } else if (functionDescriptor->returnType != NULL) {
        SemanticError("Missing return value for function");
    }

}

void processConditional(TreeNodePtr node) {
    if(node->category != IF_NODE) {
        UnexpectedNodeCategoryError(IF_NODE, node->category);
    }

    TreeNodePtr conditionNode = node->subtrees[0];
    TreeNodePtr ifCompound = node->subtrees[1];
    TreeNodePtr elseCompound = node->subtrees[2];

    char* elseLabel = nextMEPALabel();
    char* elseExitLabel = nextMEPALabel();

    TypeDescriptorPtr expressionType = processExpression(conditionNode);
    if(!equivalentTypes(expressionType, getSymbolTable()->booleanTypeDescriptor)) {
        SemanticError("Expected boolean expression");
    }
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
    TypeDescriptorPtr expressionType = processExpression(conditionNode);
    if(!equivalentTypes(expressionType, getSymbolTable()->booleanTypeDescriptor)) {
        SemanticError("Expected boolean expression");
    }
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

    TreeNodePtr specificFactorNode = node->subtrees[0];
    switch (specificFactorNode->category) {
        case VARIABLE_NODE: {
            VariablePtr variable = processVariable(specificFactorNode);
            if (variable->array) {
                addCommand("LDMV %d", variable->type->size);
            } else {
                if (variable->address) {
                    addCommand("LVLI %d, %d", variable->level, variable->displacement);
                } else {
                    addCommand("LDVL %d, %d", variable->level, variable->displacement);
                }
            }
            return variable->type;
        }
        case INTEGER_NODE: {
            int integer = processInteger(specificFactorNode);
            addCommand("LDCT %d", integer);
            return getSymbolTable()->integerTypeDescriptor;
        }
        case FUNCTION_CALL_NODE: {
            TypeDescriptorPtr functionReturnType = processFunctionCall(specificFactorNode);
            if (functionReturnType == NULL) {
                SemanticError("Void function used in expression");
            }
            return functionReturnType;
        }
            break;
        case EXPRESSION_NODE:
            return processExpression(node);
        default:
            UnexpectedChildNodeCategoryError(FACTOR_NODE, specificFactorNode->category);
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