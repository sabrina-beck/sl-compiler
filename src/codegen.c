#include "codegenp.h"


/**
 * Symbol Table
 **/
SymbolTablePtr getSymbolTable();

/**
 * Implementations
 **/
void processProgram(void *p) {
    TreeNodePtr treeRoot = (TreeNodePtr) p;


    processMainFunction(treeRoot);

    addCommand("      END ");

    printProgram();
}

void processMainFunction(TreeNodePtr node) {
    if(node->category != FUNCTION_NODE) {
        UnexpectedNodeCategoryError(FUNCTION_NODE, node->category);
    }

    processFunctionHeader(node->subtrees[0]);
    addCommand("      MAIN");

    processBlock(node->subtrees[1]);

    addCommand("      STOP");
}

void processFunction(TreeNodePtr node) {

    if(node->category != FUNCTION_NODE) {
        UnexpectedNodeCategoryError(FUNCTION_NODE, node->category);
    }

    FunctionHeaderPtr functionHeader = processFunctionHeader(node->subtrees[0]);
    SymbolTableEntryPtr entry = addFunction(getSymbolTable(), functionHeader);

    addCommand("%s:   ENFN   %d",
               entry->description.functionDescriptor->mepaLabel,
               entry->level);

    processBlock(node->subtrees[1]);

    addCommand("      RTRN   %d", entry->description.functionDescriptor->parametersSize);

    endFunctionLevel(getSymbolTable());
}

/** Function Header **/

FunctionHeaderPtr processFunctionHeader(TreeNodePtr node) {
    if(node->category != FUNCTION_HEADER_NODE) {
        UnexpectedNodeCategoryError(FUNCTION_HEADER_NODE, node->category);
    }

    TypeDescriptorPtr returnType = processFunctionReturnType(node->subtrees[0]);
    char* identifier = processIdentifier(node->subtrees[1]);
    ParameterPtr parameters = processFormalParameter(node->subtrees[2]);

    FunctionHeaderPtr function = malloc(sizeof(FunctionHeader));
    function->returnType = returnType;
    function->name = identifier;
    function->parameters = parameters;

    return function;
}

TypeDescriptorPtr processFunctionReturnType(TreeNodePtr node) {
    if(node == NULL) { // VOID function
        return NULL;
    }

    return processIdentifierAsType(node);
}

ParameterPtr processFormalParameter(TreeNodePtr node) {
    if(node == NULL) {
        return NULL;
    }

    ParameterPtr nextParameters = processFormalParameter(node->next); // process parameters in reversed order

    ParameterPtr parameters;
    switch (node->category) {
        case EXPRESSION_PARAMETER_BY_REFERENCE_NODE:
            parameters = processParameterByReference(node);
            break;
        case EXPRESSION_PARAMETER_BY_VALUE_NODE:
            parameters = processParameterByValue(node);
            break;
        case FUNCTION_PARAMETER_NODE:
            parameters = processFunctionParameter(node);
            break;
        default:
            UnexpectedChildNodeCategoryError(FUNCTION_HEADER_NODE, node->category);
    }

    return concatenateParameters(nextParameters, parameters); // again parameters in reversed order
}

ParameterPtr processParameterByReference(TreeNodePtr node) {
    if(node->category != EXPRESSION_PARAMETER_BY_REFERENCE_NODE) {
        UnexpectedNodeCategoryError(EXPRESSION_PARAMETER_BY_REFERENCE_NODE, node->category);
    }

    return processParameter(node, VARIABLE_PARAMETER);
}

ParameterPtr processParameterByValue(TreeNodePtr node) {
    if(node->category != EXPRESSION_PARAMETER_BY_VALUE_NODE) {
        UnexpectedNodeCategoryError(EXPRESSION_PARAMETER_BY_VALUE_NODE, node->category);
    }

    return processParameter(node, VALUE_PARAMETER);
}

ParameterPtr processParameter(TreeNodePtr node, ParameterPassage passage) {

    TypeDescriptorPtr type = processIdentifierAsType(node->subtrees[1]);

    TreeNodePtr identifierNode = node->subtrees[0];
    ParameterPtr previousParameter = NULL;
    while (identifierNode != NULL) {

        ParameterPtr parameter = malloc(sizeof(Parameter));
        parameter->name = processIdentifier(identifierNode);
        parameter->passage = passage;
        parameter->type = type;
        parameter->next = previousParameter;

        // it will invert the parameters order in the final structure
        previousParameter = parameter;
        identifierNode = identifierNode->next;
    }

    return previousParameter;
}

ParameterPtr processFunctionParameter(TreeNodePtr node) {
    if(node->category != FUNCTION_PARAMETER_NODE) {
        UnexpectedNodeCategoryError(FUNCTION_PARAMETER_NODE, node->category);
    }

    FunctionHeaderPtr functionHeader = processFunctionHeader(node->subtrees[0]);

    ParameterPtr parameter = malloc(sizeof(Parameter));
    parameter->name = functionHeader->name;
    parameter->passage = FUNCTION_PARAMETER;
    parameter->type = newFunctionType(functionHeader);
    parameter->next = NULL;

    return parameter;
}

/** Block **/

void processBlock(TreeNodePtr node) {
    if(node->category != BLOCK_NODE) {
        UnexpectedNodeCategoryError(BLOCK_NODE, node->category);
    }

    processLabels(node->subtrees[0]);

    processTypes(node->subtrees[1]);

    int allocatedSizeForVariables = processVariables(node->subtrees[2]);
    if(allocatedSizeForVariables > 0) {
        addCommand("      ALOC   %d", allocatedSizeForVariables);
    }

    processFunctions(node->subtrees[3]);

    processBody(node->subtrees[4], allocatedSizeForVariables);

    if(allocatedSizeForVariables > 0) {
        addCommand("      DLOC   %d", allocatedSizeForVariables);
    }
}

void processLabels(TreeNodePtr node) {
    if(node == NULL) {
        return;
    }

    if(node->category != LABELS_NODE) {
        UnexpectedNodeCategoryError(LABELS_NODE, node->category);
    }

    TreeNodePtr identifierNode = node->subtrees[0];
    while(identifierNode != NULL) {

        char* identifier = processIdentifier(identifierNode);
        addLabel(getSymbolTable(), identifier);

        identifierNode = identifierNode->next;
    }
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

    SymbolTableEntryPtr typeEntry = newType(getFunctionLevel(), identifier, type);
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
        SymbolTableEntryPtr variableEntry = newVariable(getFunctionLevel(), identifier, *displacement, type);
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
        processFunction(functionNode);
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

TypeDescriptorPtr processIdentifierAsType(TreeNodePtr node) {

    char* identifier = processIdentifier(node);
    SymbolTableEntryPtr entry = findIdentifier(getSymbolTable(), identifier);

    if(entry->category != TYPE_SYMBOL) {
        UnexpectedSymbolEntryCategoryError(TYPE_SYMBOL, entry->category);
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
        UnexpectedSymbolEntryCategoryError(TYPE_SYMBOL, entry->category);
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
        UnexpectedSymbolEntryCategoryError(LABEL_SYMBOL, symbolTableEntry->category);
    }

    if(symbolTableEntry->description.labelDescriptor->defined) {
        SemanticError("Label already used");
    }

    LabelDescriptorPtr labelDescriptor = symbolTableEntry->description.labelDescriptor;
    labelDescriptor->defined = true;

    // since there are only statements at this level, the current activation record displacement on the stack
    // will always be equal to the size of its alocated variables
    addCommand("%s:   ENLB   %d,%d        %s:", labelDescriptor->mepaLabel, symbolTableEntry->level, allocatedSizeForVariables, identifier);
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

    Variable variable = processVariable(variableNode);
    TypeDescriptorPtr exprType = processExpression(expressionNode);

    if(!equivalentTypes(exprType, variable.type)) {
        SemanticError("Trying to assign value to variable of incompatible type");
    }
    switch (variable.category) {
        case ARRAY:
            addCommand("      STMV   %d", variable.type->size);
            break;
        case ADDRESS:
            addCommand("      STVI   %d,%d", variable.level, variable.displacement);
            break;
        case VALUE:
            addCommand("      STVL   %d,%d", variable.level, variable.displacement);
            break;
        case CONSTANT:
            addCommand("      LDCT   %d", variable.value);
            break;
    }
}

// if it is an indexed array, the position address will be on top of the stack (exec time)
Variable processVariable(TreeNodePtr node) {
    if(node->category != VARIABLE_NODE) {
        UnexpectedNodeCategoryError(VARIABLE_NODE, node->category);
    }

    TreeNodePtr identifierNode = node->subtrees[0];
    char* identifier = processIdentifier(identifierNode);
    SymbolTableEntryPtr entry = findIdentifier(getSymbolTable(), identifier);

    Variable variable;
    variable.level = entry->level;

    TypeDescriptorPtr variableDeclarationType;
    switch (entry->category) {
        case VARIABLE_SYMBOL: {
            variable.category = VALUE;
            variable.displacement = entry->description.variableDescriptor->displacement;
            variableDeclarationType = entry->description.variableDescriptor->type;
        }
            break;
        case PARAMETER_SYMBOL: {
            ParameterPassage passage = entry->description.parameterDescriptor->parameterPassage;
            switch (passage) {
                case VALUE_PARAMETER:
                    variable.category = ADDRESS;
                    break;
                case VARIABLE_PARAMETER:
                    variable.category = VALUE;
                    break;
                default:
                    SemanticError("Unexpected parameter passage type");
            }
            variable.displacement = entry->description.parameterDescriptor->displacement;
            variableDeclarationType = entry->description.parameterDescriptor->type;
        }
            break;
        case CONSTANT_SYMBOL:
            variable.category = CONSTANT;
            variable.value = entry->description.constantDescriptor->value;
            variableDeclarationType = entry->description.constantDescriptor->type;
            break;
        default:
            SymbolEntryCategoryError("variable", entry->category);
    }

    TreeNodePtr arrayIndexNode = node->subtrees[1];
    TypeDescriptorPtr arrayPositionType = NULL;
    if(entry->category == ARRAY_TYPE) {
        arrayPositionType = processArrayIndexList(arrayIndexNode, variable, variableDeclarationType);
    }
    if(arrayIndexNode != NULL && variableDeclarationType->category != ARRAY_TYPE) {
        SemanticError("Trying to index non array variable");
    }

    if(arrayPositionType != NULL) {
        variable.type = arrayPositionType;
        variable.category = ARRAY;
    } else {
        variable.type = variableDeclarationType;
    }

    return variable;
}

TypeDescriptorPtr processArrayIndexList(TreeNodePtr node, Variable variable, TypeDescriptorPtr variableTypeDescriptor) {
    switch (variable.category) {
        case ADDRESS:
            addCommand("      LDVL   %d,%d", variable.level, variable.displacement);
            break;
        case VALUE:
            addCommand("      LADR   %d,%d", variable.level, variable.displacement);
            break;
        case ARRAY:
            //FIXME parameters we won't receive an array here
            break;
        case CONSTANT:
            if(node != NULL) {
                SemanticError("Constant can't have index");
            }
            return NULL;
            break;
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
    addCommand("      INDX   %d", arrayTypeDescriptor->size);

    return arrayTypeDescriptor->description.arrayDescriptor->elementType;
}

TypeDescriptorPtr processFunctionCall(TreeNodePtr node) {
    if(node->category != FUNCTION_CALL_NODE) {
        UnexpectedNodeCategoryError(FUNCTION_CALL_NODE, node->category);
    }

    TreeNodePtr identifierNode = node->subtrees[0];
    char* identifier = processIdentifier(identifierNode);
    SymbolTableEntryPtr functionEntry = findIdentifier(getSymbolTable(), identifier);

    TreeNodePtr expressionListNode = node->subtrees[1];

    switch (functionEntry->category) {
        case PARAMETER_SYMBOL: {
            ParameterDescriptorPtr parameterDescriptor = functionEntry->description.parameterDescriptor;
            if (parameterDescriptor->type->category != FUNCTION_TYPE) {
                SemanticError("Expected function as parameter");
            }
            List* expectedParameters = parameterDescriptor->type->description.functionTypeDescriptor->params;
            processExpressionListAsParameters(expressionListNode, expectedParameters);
            addCommand("      CPFN   %d,%d,%d", functionEntry->level, parameterDescriptor->displacement, getFunctionLevel());

            return parameterDescriptor->type->description.functionTypeDescriptor->returnType;
        }
        case FUNCTION_SYMBOL: {
            FunctionDescriptorPtr functionDescriptor = functionEntry->description.functionDescriptor;

            processExpressionListAsParameters(expressionListNode, functionDescriptor->params);
            addCommand("      CFUN   %s,%d", functionDescriptor->mepaLabel, getFunctionLevel());

            return functionDescriptor->returnType;
        }
        case PSEUDO_FUNCTION_SYMBOL: {
            TreeNodePtr currentExpr = expressionListNode;
            while (currentExpr != NULL) {
                switch (functionEntry->description.pseudoFunction) {
                    case READ: {
                        addCommand("      READ");
                        TreeNodePtr variableNode = getVariableExpression(currentExpr);
                        Variable variable = processVariable(variableNode);
                        switch (variable.category) {
                            case ARRAY:
                                addCommand("      STMV   1");
                                break;
                            case ADDRESS:
                                addCommand("      STVI   %d,%d", variable.level, variable.displacement);
                                break;
                            case VALUE:
                                addCommand("      STVL   %d,%d", variable.level, variable.displacement);
                                break;
                            case CONSTANT:
                                SemanticError("Can't read boolean value");
                                break;
                        }
                    }
                        break;
                    case WRITE: {
                        processExpression(currentExpr);
                        addCommand("      PRNT");
                    }
                        break;
                }
                currentExpr = currentExpr->next;
            }

            return NULL;
        }
        default:
            SymbolEntryCategoryError("function call", functionEntry->category);
    }
}

void processExpressionListAsParameters(TreeNodePtr node, List* expectedParams) {
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
                Variable variable = processVariable(variableNode);

                if(!equivalentTypes(expectedParameterDescriptor->type, variable.type)) {
                    SemanticError("Wrong parameter type on function");
                }

                switch (variable.category) {
                    case ARRAY:
                        // process variable already left the address on top of the stack
                        break;
                    case ADDRESS:
                        addCommand("      LDVL   %d,%d", variable.level, variable.displacement);
                        break;
                    case VALUE:
                        addCommand("      LADR   %d,%d", variable.level, variable.displacement);
                        break;
                    case CONSTANT:
                        addCommand("      LDCT   %d", variable.value);
                        break;
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
                        addCommand("      LGAD   %s,%d", functionDescriptor->mepaLabel, variableEntry->level - 1);
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
                        addCommand("      LDVL   %s,%d", variableEntry->level, parameterDescriptor->displacement);
                        // load the value of the base register D[k]
                        addCommand("      LDVL   %s,%d", variableEntry->level, parameterDescriptor->displacement + 1);
                        // load the level k
                        addCommand("      LDVL   %s,%d", variableEntry->level, parameterDescriptor->displacement + 2);
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
        UnexpectedSymbolEntryCategoryError(LABEL_SYMBOL, labelEntry->category);
    }

    LabelDescriptorPtr labelDescriptor = labelEntry->description.labelDescriptor;

    addCommand("      JUMP   %s         goto %s", labelDescriptor->mepaLabel, identifier);
}

void processReturn(TreeNodePtr node) {
    if(node->category != RETURN_NODE) {
        UnexpectedNodeCategoryError(RETURN_NODE, node->category);
    }

    FunctionDescriptorPtr functionDescriptor = findCurrentFunctionDescriptor(getSymbolTable(), getFunctionLevel());
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
            addCommand("      STVL   %d,%d", getFunctionLevel(), functionDescriptor->returnDisplacement);
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
    addCommand("      JMPF   %s        if", elseLabel);

    processCompound(ifCompound);

    if(elseCompound != NULL) {
        addCommand("      JUMP   %s", elseExitLabel);

        addCommand("%s:   NOOP             else", elseLabel);
        processCompound(elseCompound);

        addCommand("%s:   NOOP             end if", elseExitLabel);
    } else {
        addCommand("%s:   NOOP             end if", elseLabel);
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

    addCommand("%s:   NOOP             while", conditionLabel);
    TypeDescriptorPtr expressionType = processExpression(conditionNode);
    if(!equivalentTypes(expressionType, getSymbolTable()->booleanTypeDescriptor)) {
        SemanticError("Expected boolean expression");
    }
    addCommand("      JMPF   %s", exitLabel);

    processCompound(compoundNode);
    addCommand("      JUMP   %s", conditionLabel);

    addCommand("%s:   NOOP             end while", exitLabel);

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
    if (relationalOperatorNode == NULL) {
        return firstExprType;
    }

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
            Variable variable = processVariable(specificFactorNode);
            switch (variable.category) {
                case ARRAY:
                    addCommand("      LDMV   %d", variable.type->size);
                    break;
                case ADDRESS:
                    addCommand("      LVLI   %d, %d", variable.level, variable.displacement);
                    break;
                case VALUE:
                    addCommand("      LDVL   %d,%d", variable.level, variable.displacement);
                    break;
                case CONSTANT: {
                    addCommand("      LDCT   %d", variable.value);
                }
                    break;
            }
            return variable.type;
        }
        case INTEGER_NODE: {
            int integer = processInteger(specificFactorNode);
            addCommand("      LDCT   %d", integer);
            return getSymbolTable()->integerTypeDescriptor;
        }
        case FUNCTION_CALL_NODE: {
            TypeDescriptorPtr functionReturnType = processFunctionCall(specificFactorNode);
            if (functionReturnType == NULL) {
                SemanticError("Void function used in expression");
            }
            return functionReturnType;
        }
        case EXPRESSION_NODE:
            return processExpression(specificFactorNode);
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
            addCommand("      LEQU");
            break;
        case LESS_NODE:
            addCommand("      LESS");
            break;
        case EQUAL_NODE:
            addCommand("      EQUA");
            break;
        case DIFFERENT_NODE:
            addCommand("      DIFF");
            break;
        case GREATER_OR_EQUAL_NODE:
            addCommand("      GEQU");
            break;
        case GREATER_NODE:
            addCommand("      GRTR");
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
            addCommand("      ADDD");
            return getSymbolTable()->integerTypeDescriptor;
        case MINUS_NODE:
            addCommand("      SUBT");
            return getSymbolTable()->integerTypeDescriptor;
        case OR_NODE:
            addCommand("      LORR");
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
            addCommand("      NEGT");
            return getSymbolTable()->integerTypeDescriptor;
        case NOT_NODE:
            addCommand("      LNOT");
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
            addCommand("      MULT");
            return getSymbolTable()->integerTypeDescriptor;
        case DIV_NODE:
            addCommand("      DIVI");
            return getSymbolTable()->integerTypeDescriptor;
        case AND_NODE:
            addCommand("      LAND");
            return getSymbolTable()->booleanTypeDescriptor;
        default:
            UnexpectedChildNodeCategoryError(MULTIPLICATIVE_OPERATOR_NODE, operatorNode->category);
    }
}

/** Semantic Errors handling functions **/

void UnexpectedSymbolEntryCategoryError(SymbolTableCategory expected, SymbolTableCategory gotten) {
    char message[100];
    sprintf(message, "Expected %s, but got %s", getSymbolTableCategoryName(expected), getSymbolTableCategoryName(gotten));
    SemanticError(message);
}

void SymbolEntryCategoryError(char* expected, SymbolTableCategory gotten) {
    char message[100];
    sprintf(message, "Expected %s, but got %s", expected, getSymbolTableCategoryName(gotten));
    SemanticError(message);
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
        printf("%s\n", command);
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