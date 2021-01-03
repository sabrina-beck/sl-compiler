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
}

void processMainFunction(TreeNodePtr node) {
    if(node->category != FUNCTION_NODE) {
        UnexpectedNodeCategoryError(FUNCTION_NODE, node->category);
    }

    FunctionHeaderPtr functionHeader = processFunctionHeader(node->subtrees[0]);
    if(functionHeader->returnType != NULL) {
        SemanticError("Main function should be void");
    }
    if(functionHeader->parameters != NULL) {
        SemanticError("Main function shouldn't have any parameters");
    }
    addMainFunction(getSymbolTable());

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

    addCommand("L%d:   ENFN   %d         %s",
               entry->description.functionDescriptor->headerMepaLabel,
               entry->level,
               entry->identifier);
    processBlock(node->subtrees[1]);

    addCommand("L%d:   NOOP             ", entry->description.functionDescriptor->returnMepaLabel);
    addCommand("      RTRN   %d        end function", entry->description.functionDescriptor->parametersSize);

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

    processVariables(node->subtrees[2]);

    FunctionDescriptorPtr functionDescriptor = findCurrentFunctionDescriptor(getSymbolTable());

    if(functionDescriptor->variablesDisplacement > 0) {
        addCommand("      ALOC   %d", functionDescriptor->variablesDisplacement);
    }

    TreeNodePtr functionsNode = node->subtrees[3];
    if(functionsNode != NULL) {
        if(functionDescriptor->bodyMepaLabel <= 0) { // generate Main functions label only if necessary
            functionDescriptor->bodyMepaLabel = nextMEPALabel();
        }
        addCommand("      JUMP   L%d", functionDescriptor->bodyMepaLabel);
    }
    processFunctions(functionsNode);


    if(functionsNode != NULL) {
        addCommand("L%d:   NOOP             body", functionDescriptor->bodyMepaLabel);
    }
    processBody(node->subtrees[4]);

    if(functionDescriptor->variablesDisplacement > 0) {
        addCommand("      DLOC   %d", functionDescriptor->variablesDisplacement);
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

    char* identifier = processIdentifier(node->subtrees[0]);
    TypeDescriptorPtr type = processType(node->subtrees[1]);
    addType(getSymbolTable(), identifier, type);
}

void processVariables(TreeNodePtr node) {
    if(node == NULL) {
        return;
    }

    if(node->category != VARIABLES_NODE) {
        UnexpectedNodeCategoryError(VARIABLES_NODE, node->category);
    }

    TreeNodePtr variableDeclarationNode = node->subtrees[0];
    while (variableDeclarationNode != NULL) {

        processVariableDeclaration(variableDeclarationNode);

        variableDeclarationNode = variableDeclarationNode->next;
    }
}

void processVariableDeclaration(TreeNodePtr node) {
    if(node->category != DECLARATION_NODE) {
        UnexpectedNodeCategoryError(DECLARATION_NODE, node->category);
    }

    TypeDescriptorPtr type = processType(node->subtrees[1]);

    TreeNodePtr identifierNode = node->subtrees[0];
    while(identifierNode != NULL) {
        char* identifier = processIdentifier(identifierNode);
        addVariable(getSymbolTable(), identifier, type);
        identifierNode = identifierNode->next;
    }
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

    char* identifier = processIdentifier(node->subtrees[0]);
    SymbolTableEntryPtr entry = findIdentifier(getSymbolTable(), identifier);

    if(entry->category != TYPE_SYMBOL) {
        UnexpectedSymbolEntryCategoryError(TYPE_SYMBOL, entry->category);
    }

    TypeDescriptorPtr elementType = entry->description.typeDescriptor;
    TypeDescriptorPtr arrayType = processArraySizeDeclaration(node->subtrees[1], elementType);

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
    int dimension = processInteger(node->subtrees[0]);

    if(subArrayType != NULL) {
        return newArrayType(dimension, subArrayType);
    }
    return newArrayType(dimension, elementType);
}

/* Body */

void processBody(TreeNodePtr node) {
    if(node->category != BODY_NODE) {
        UnexpectedNodeCategoryError(BODY_NODE, node->category);
    }

    TreeNodePtr statementNode = node->subtrees[0];
    while (statementNode != NULL) {
        processStatement(statementNode);
        statementNode = statementNode->next;
    }
}

void processStatement(TreeNodePtr node) {
    if(node->category != STATEMENT_NODE) {
        UnexpectedNodeCategoryError(STATEMENT_NODE, node->category);
    }

    TreeNodePtr labelNode = node->subtrees[0];
    TreeNodePtr unlabeledStatementNode = node->subtrees[1];
    if(labelNode->category != LABEL_NODE) {
        unlabeledStatementNode = labelNode;
        labelNode = NULL;
    }

    processLabel(labelNode);
    processUnlabeledStatement(unlabeledStatementNode);

}

void processLabel(TreeNodePtr node) {
    if(node == NULL) {
        return;
    }

    if(node->category != LABEL_NODE) {
        UnexpectedNodeCategoryError(LABEL_NODE, node->category);
    }

    char* identifier = processIdentifier(node->subtrees[0]);
    SymbolTableEntryPtr symbolTableEntry = findIdentifier(getSymbolTable(), identifier);

    if(symbolTableEntry == NULL) {
        UndeclaredLabelError(identifier);
    }

    if(symbolTableEntry->category != LABEL_SYMBOL) {
        UnexpectedSymbolEntryCategoryError(LABEL_SYMBOL, symbolTableEntry->category);
    }

    if(symbolTableEntry->description.labelDescriptor->defined) {
        LabelAlreadyDefinedError(identifier);
    }

    LabelDescriptorPtr labelDescriptor = symbolTableEntry->description.labelDescriptor;
    labelDescriptor->defined = true;

    // since there are only statements at this level, the current activation record displacement on the stack
    // will always be equal to the size of its alocated variables
    FunctionDescriptorPtr functionDescriptor = findCurrentFunctionDescriptor(getSymbolTable());
    addCommand("L%d:   ENLB   %d,%d        %s:",
               labelDescriptor->mepaLabel,
               symbolTableEntry->level,
               functionDescriptor->variablesDisplacement,
               identifier);
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

    Value value = processValue(node->subtrees[0]);
    TypeDescriptorPtr exprType = processExpression(node->subtrees[1]);

    if(!equivalentTypes(exprType, value.type)) {
        SemanticError("Trying to assign value to variable of incompatible type");
    }

    switch (value.category) {
        case ARRAY_VALUE:
        case ARRAY_REFERENCE:
            addCommand("      STMV   %d", value.type->size);
            break;
        case REFERENCE:
            addCommand("      STVI   %d,%d", value.level, value.content.displacement);
            break;
        case VALUE:
            addCommand("      STVL   %d,%d", value.level, value.content.displacement);
            break;
        case CONSTANT:
            SemanticError("Constants can't be assigned");
            break;
    }
}

// if it is an indexed array, the position address will be on top of the stack (exec time)
Value processValue(TreeNodePtr node) {
    if(node->category != VALUE_NODE) {
        UnexpectedNodeCategoryError(VALUE_NODE, node->category);
    }

    char* identifier = processIdentifier(node->subtrees[0]);
    SymbolTableEntryPtr entry = findIdentifier(getSymbolTable(), identifier);

    Value value = valueFromEntry(entry);
    switch(value.category) {
        case CONSTANT:
        case REFERENCE:
        case VALUE: {
            return value;
        }
        case ARRAY_VALUE:
        case ARRAY_REFERENCE: {
            loadArrayBaseAddress(value);

            TreeNodePtr arrayIndexNode = node->subtrees[1];
            processArrayIndexList(arrayIndexNode, &value);

            return value;
        }
    }
}

void loadArrayBaseAddress(Value value) {
    switch (value.category) {
        case ARRAY_VALUE:
            addCommand("      LADR   %d,%d", value.level, value.content.displacement);
            break;
        case ARRAY_REFERENCE:
            addCommand("      LDVL   %d,%d", value.level, value.content.displacement);
            break;
        default:
            return;
    }
}

// it modifies value.type
void processArrayIndexList(TreeNodePtr node, Value* value) {
    if(node == NULL) {
        return;
    }

    TreeNodePtr currentIndexNode = node;
    while (currentIndexNode != NULL) {
        processArrayIndex(currentIndexNode, value);
        currentIndexNode = currentIndexNode->next;
    }
}

void processArrayIndex(TreeNodePtr node, Value* value) {
    if(node->category != ARRAY_INDEX_NODE) {
        UnexpectedNodeCategoryError(ARRAY_INDEX_NODE, node->category);
    }

    if(value->type->category != ARRAY_TYPE) {
        SemanticError("Expected array type to process array index");
    }

    TypeDescriptorPtr exprType = processExpression(node->subtrees[0]);

    if(!equivalentTypes(exprType, getSymbolTable()->integerTypeDescriptor)) {
        SemanticError("Index should be an integer");
    }

    TypeDescriptorPtr arrayElementType = value->type->description.arrayDescriptor->elementType;
    addCommand("      INDX   %d", arrayElementType->size);

    value->type = arrayElementType;
}

TypeDescriptorPtr processFunctionCall(TreeNodePtr node) {
    if(node->category != FUNCTION_CALL_NODE) {
        UnexpectedNodeCategoryError(FUNCTION_CALL_NODE, node->category);
    }

    char* identifier = processIdentifier(node->subtrees[0]);
    SymbolTableEntryPtr functionEntry = findIdentifier(getSymbolTable(), identifier);

    switch (functionEntry->category) {
        case PARAMETER_SYMBOL: {
            return processFunctionParameterCall(node, functionEntry);
        }
        case FUNCTION_SYMBOL: {
            return processRegularFunctionCall(node, functionEntry);
        }
        case PSEUDO_FUNCTION_SYMBOL: {
            return processPseudoFunctionCall(node, functionEntry);
        }
        default:
            SymbolEntryCategoryError("function call", functionEntry->category);
    }
}

TypeDescriptorPtr processFunctionParameterCall(TreeNodePtr node, SymbolTableEntryPtr functionEntry) {

    ParameterDescriptorPtr parameterDescriptor = functionEntry->description.parameterDescriptor;
    TypeDescriptorPtr parameterType = parameterDescriptor->type;

    if (parameterType->category != FUNCTION_TYPE) {
        SemanticError("Expected function as parameter");
    }

    TypeDescriptorPtr returnType = parameterType->description.functionTypeDescriptor->returnType;
    if(returnType!= NULL && returnType->size > 0) {
        addCommand("      ALOC   %d        result", returnType->size);
    }

    ParameterDescriptorsListPtr expectedParameters =
            parameterType->description.functionTypeDescriptor->parameters;
    processArgumentsList(node->subtrees[1], expectedParameters);

    addCommand("      CPFN   %d,%d,%d",
               functionEntry->level,
               parameterDescriptor->displacement,
               getFunctionLevel());

    return parameterDescriptor->type->description.functionTypeDescriptor->returnType;
}

TypeDescriptorPtr processRegularFunctionCall(TreeNodePtr node, SymbolTableEntryPtr functionEntry) {
    FunctionDescriptorPtr functionDescriptor = functionEntry->description.functionDescriptor;

    TypeDescriptorPtr returnType = functionDescriptor->returnType;
    if(returnType!= NULL && returnType->size > 0) {
        addCommand("      ALOC   %d        result", returnType->size);
    }

    processArgumentsList(node->subtrees[1], functionDescriptor->parameters);
    addCommand("      CFUN   L%d,%d", functionDescriptor->headerMepaLabel, getFunctionLevel());

    return functionDescriptor->returnType;
}

TypeDescriptorPtr processPseudoFunctionCall(TreeNodePtr node, SymbolTableEntryPtr functionEntry) {
    TreeNodePtr argumentNode = node->subtrees[1];

    switch (functionEntry->description.pseudoFunction) {
        case READ: {
            processReadFunctionCall(argumentNode);
            break;
        }
        case WRITE: {
            processWriteFunctionCall(argumentNode);
            break;
        }
    }

    return NULL;
}

void processReadFunctionCall(TreeNodePtr argumentNode) {
    if(argumentNode == NULL) {
        return;
    }

    addCommand("      READ");

    TreeNodePtr valueNode = getVariableExpression(argumentNode);
    Value value = processValue(valueNode);

    if(value.type->size > 1) {
        SemanticError("Can't read multiple values at a time");
    }

    switch (value.category) {
        case ARRAY_VALUE:
        case ARRAY_REFERENCE:
            addCommand("      STMV   1");
            break;
        case REFERENCE:
            addCommand("      STVI   %d,%d", value.level, value.content.displacement);
            break;
        case VALUE:
            addCommand("      STVL   %d,%d", value.level, value.content.displacement);
            break;
        case CONSTANT:
            SemanticError("Can't read boolean value");
            break;
    }

    processReadFunctionCall(argumentNode->next);
}

void processWriteFunctionCall(TreeNodePtr argumentNode) {
    if(argumentNode == NULL) {
        return;
    }

    processExpression(argumentNode);
    addCommand("      PRNT");

    processWriteFunctionCall(argumentNode->next);
}

void processArgumentsList(TreeNodePtr node, ParameterDescriptorsListPtr parameters) {

    TreeNodePtr currentNode = node;
    ParameterDescriptorsListPtr currentParameter = parameters;
    while (currentParameter != NULL && currentNode != NULL) {

        switch (currentParameter->descriptor->parameterPassage) {
            case VALUE_PARAMETER: {
                processArgumentByValue(currentParameter->descriptor, currentNode);
                break;
            }
            case VARIABLE_PARAMETER: {
                processArgumentByReference(currentParameter->descriptor, currentNode);
                break;
            }
            case FUNCTION_PARAMETER: {
                processArgumentByFunctionAsParameter(currentParameter->descriptor, currentNode);
                break;
            }
        }

        currentParameter = currentParameter->next;
        currentNode = currentNode->next;
    }

    if(currentParameter != NULL) {
        SemanticError("Missing parameters for function call");
    }

    if(currentNode != NULL) {
        SemanticError("Too many parameters for function call");
    }
}

void processArgumentByValue(ParameterDescriptorPtr expectedParameter, TreeNodePtr node) {
    TypeDescriptorPtr expressionType = processExpression(node);

    if(!equivalentTypes(expectedParameter->type, expressionType)) {
        SemanticError("Wrong parameter type on function");
    }
}

// Arguments by reference can only be a variable or array position
void processArgumentByReference(ParameterDescriptorPtr expectedParameter, TreeNodePtr node) {
    TreeNodePtr valueNode = getVariableExpression(node);
    Value value = processValue(valueNode);

    if(!equivalentTypes(expectedParameter->type, value.type)) {
        SemanticError("Wrong parameter type on function");
    }

    switch (value.category) {
        case ARRAY_VALUE:
        case ARRAY_REFERENCE:
            // process value already left the address on top of the stack
            break;
        case REFERENCE:
            addCommand("      LDVL   %d,%d", value.level, value.content.displacement);
            break;
        case VALUE:
            addCommand("      LADR   %d,%d", value.level, value.content.displacement);
            break;
        case CONSTANT:
            SemanticError("Can't pass constant by reference");
            break;
    }
}

void processArgumentByFunctionAsParameter(ParameterDescriptorPtr expectedParameter, TreeNodePtr node) {

    TreeNodePtr valueNode = getVariableExpression(node);

    TreeNodePtr identifierNode = valueNode->subtrees[0];
    char* identifier = processIdentifier(identifierNode);
    SymbolTableEntryPtr valueEntry = findIdentifier(getSymbolTable(), identifier);

    switch (valueEntry->category) {
        case FUNCTION_SYMBOL: {
            processDeclaredFunctionAsArgument(expectedParameter, valueEntry);
            break;
        }
        case PARAMETER_SYMBOL: {
            processFunctionParameterAsArgument(expectedParameter, valueEntry);
            break;
        }
        default:
            SemanticError("Expected function as parameter");
    }

    TreeNodePtr arrayIndexNode = valueNode->subtrees[1];
    if(arrayIndexNode != NULL) {
        SemanticError("Trying to index function identifier");
    }
}

void processDeclaredFunctionAsArgument(ParameterDescriptorPtr expectedParameter, SymbolTableEntryPtr valueEntry) {

    if(!equivalentTypes(expectedParameter->type, valueEntry->description.functionDescriptor->functionType)) {
        SemanticError("Wrong parameter type on function");
    }

    addCommand("      LGAD   L%d,%d",
               valueEntry->description.functionDescriptor->headerMepaLabel,
               valueEntry->level - 1);
}

void processFunctionParameterAsArgument(ParameterDescriptorPtr expectedParameter, SymbolTableEntryPtr valueEntry) {

    ParameterDescriptorPtr argumentDescriptor = valueEntry->description.parameterDescriptor;

    if (argumentDescriptor->type->category != FUNCTION_TYPE) {
        SemanticError("Expected function as parameter");
    }

    if(!equivalentTypes(expectedParameter->type, argumentDescriptor->type)) {
        SemanticError("Wrong parameter type on function");
    }

    // generalized address is already on the stack as the current function parameter
    // load the MEPA address
    addCommand("      LDVL   %s,%d", valueEntry->level, argumentDescriptor->displacement);
    // load the value of the base register D[k]
    addCommand("      LDVL   %s,%d", valueEntry->level, argumentDescriptor->displacement + 1);
    // load the level k
    addCommand("      LDVL   %s,%d", valueEntry->level, argumentDescriptor->displacement + 2);
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

    TreeNodePtr valueNode = factorNode->subtrees[0];

    if(valueNode->category == EXPRESSION_NODE) {
        valueNode = getVariableExpression(valueNode);
    }

    if(valueNode->category != VALUE_NODE) {
        UnexpectedNodeCategoryError(VALUE_NODE, valueNode->category);
    }

    return valueNode;
}

void processGoto(TreeNodePtr node) {
    if(node->category != GOTO_NODE) {
        UnexpectedNodeCategoryError(GOTO_NODE, node->category);
    }

    char* identifier = processIdentifier(node->subtrees[0]);

    SymbolTableEntryPtr labelEntry = findIdentifier(getSymbolTable(), identifier);
    if(labelEntry->category != LABEL_SYMBOL) {
        UnexpectedSymbolEntryCategoryError(LABEL_SYMBOL, labelEntry->category);
    }

    addCommand("      JUMP   L%d         goto %s",
               labelEntry->description.labelDescriptor->mepaLabel,
               identifier);
}

void processReturn(TreeNodePtr node) {
    if(node->category != RETURN_NODE) {
        UnexpectedNodeCategoryError(RETURN_NODE, node->category);
    }

    FunctionDescriptorPtr functionDescriptor = findCurrentFunctionDescriptor(getSymbolTable());
    if(functionDescriptor == NULL) {
        SemanticError("Unexpected error, can't find current function descriptor");
    }

    TreeNodePtr expressionNode = node->subtrees[0];
    if(expressionNode != NULL && functionDescriptor->returnType != NULL) {
        processReturnWithValue(expressionNode, functionDescriptor);
    } else if (expressionNode == NULL && functionDescriptor->returnType == NULL) {
        addCommand("      JUMP   L%d", functionDescriptor->returnMepaLabel);
    } else if(functionDescriptor->returnType == NULL) {
        SemanticError("Can't return value for void function");
    } else if(expressionNode == NULL) {
        SemanticError("Missing return value for function");
    }
}

void processReturnWithValue(TreeNodePtr expressionNode, FunctionDescriptorPtr functionDescriptor) {
    if(functionDescriptor->returnType->size > 1) {
        addCommand("      LADR   %d,%d", getFunctionLevel(), functionDescriptor->returnDisplacement);
    }

    TypeDescriptorPtr expressionType = processExpression(expressionNode);
    if (!equivalentTypes(expressionType, functionDescriptor->returnType)) {
        SemanticError("Expression with type different than function return type");
    }

    if(expressionType->size == 1) {
        addCommand("      STVL   %d,%d", getFunctionLevel(), functionDescriptor->returnDisplacement);
    } else {
        addCommand("      STMV   %d,%d", getFunctionLevel(), functionDescriptor->returnDisplacement);
    }
    addCommand("      JUMP   L%d", functionDescriptor->returnMepaLabel);
}

void processConditional(TreeNodePtr node) {
    if(node->category != IF_NODE) {
        UnexpectedNodeCategoryError(IF_NODE, node->category);
    }

    TreeNodePtr conditionNode = node->subtrees[0];
    TreeNodePtr ifCompound = node->subtrees[1];
    TreeNodePtr elseCompound = node->subtrees[2];

    int elseLabel = nextMEPALabel();
    int elseExitLabel = nextMEPALabel();

    TypeDescriptorPtr expressionType = processExpression(conditionNode);
    if(!equivalentTypes(expressionType, getSymbolTable()->booleanTypeDescriptor)) {
        SemanticError("Expected boolean expression");
    }
    addCommand("      JMPF   L%d        if", elseLabel);

    processCompound(ifCompound);

    if(elseCompound != NULL) {
        addCommand("      JUMP   L%d", elseExitLabel);

        addCommand("L%d:   NOOP             else", elseLabel);
        processCompound(elseCompound);

        addCommand("L%d:   NOOP             end if", elseExitLabel);
    } else {
        addCommand("L%d:   NOOP             end if", elseLabel);
    }
}

void processRepetitive(TreeNodePtr node) {
    if(node->category != WHILE_NODE) {
        UnexpectedNodeCategoryError(WHILE_NODE, node->category);
    }

    TreeNodePtr conditionNode = node->subtrees[0];
    TreeNodePtr compoundNode = node->subtrees[1];

    int conditionLabel = nextMEPALabel();
    int exitLabel = nextMEPALabel();

    addCommand("L%d:   NOOP             while", conditionLabel);
    TypeDescriptorPtr expressionType = processExpression(conditionNode);
    if(!equivalentTypes(expressionType, getSymbolTable()->booleanTypeDescriptor)) {
        SemanticError("Expected boolean expression");
    }
    addCommand("      JMPF   L%d", exitLabel);

    processCompound(compoundNode);
    addCommand("      JUMP   L%d", conditionLabel);

    addCommand("L%d:   NOOP             end while", exitLabel);

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

    TreeNodePtr firstExprNode = node->subtrees[0];
    TreeNodePtr relationalOperatorNode = node->subtrees[1];
    TreeNodePtr binaryOpExprNode = node->subtrees[2];

    if(relationalOperatorNode == NULL) {
        return routeExpressionSubtree(firstExprNode);
    }

    TypeDescriptorPtr firstExprType = routeExpressionSubtree(firstExprNode);
    TypeDescriptorPtr secondExprType = processBinaryOpExpression(binaryOpExprNode);
    TypeDescriptorPtr operatorType = processRelationalOperator(relationalOperatorNode);

    if(!equivalentTypes(firstExprType, secondExprType)) {
        SemanticError("Expressions of incompatible type");
    }

    return operatorType;
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
    TreeNodePtr operatorNode = node->subtrees[1];
    TreeNodePtr binaryOpExpressionNode = node->subtrees[2];

    if(operatorNode == NULL) {
        return processTerm(termNode);
    }

    TypeDescriptorPtr termType = processTerm(termNode);
    TypeDescriptorPtr binaryOpExpressionType = processBinaryOpExpression(binaryOpExpressionNode);
    TypeDescriptorPtr operatorType = processAdditiveOperator(operatorNode);

    if(!equivalentTypes(termType, operatorType) ||
       !equivalentTypes(binaryOpExpressionType, operatorType)) {
        SemanticError("Expression's terms have incompatible types");
    }

    return operatorType;

}

TypeDescriptorPtr processUnopExpression(TreeNodePtr node) {
    if(node->category != UNARY_OPERATOR_EXPRESSION_NODE) {
        UnexpectedNodeCategoryError(UNARY_OPERATOR_EXPRESSION_NODE, node->category);
    }

    TreeNodePtr unaryOperatorNode = node->subtrees[0];
    TreeNodePtr termNode = node->subtrees[1];
    TreeNodePtr additiveOperatorNode = node->subtrees[2];
    TreeNodePtr binaryOpExpression = node->subtrees[3];

    if(additiveOperatorNode == NULL) {
        TypeDescriptorPtr termType = processTerm(termNode);
        TypeDescriptorPtr operatorType = processUnaryOperator(unaryOperatorNode);

        if(!equivalentTypes(termType, operatorType)) {
            SemanticError("Expression's term type incompatible with unary operator");
        }

        return operatorType;
    }



    TypeDescriptorPtr termType = processTerm(termNode);
    TypeDescriptorPtr operatorType = processUnaryOperator(unaryOperatorNode);

    TypeDescriptorPtr binaryOpExpressionType = processBinaryOpExpression(binaryOpExpression);

    TypeDescriptorPtr additiveOperatorType = processAdditiveOperator(additiveOperatorNode);

    if(!equivalentTypes(termType, operatorType) ||
       !equivalentTypes(termType, additiveOperatorType) ||
       !equivalentTypes(binaryOpExpressionType, additiveOperatorType)) {
        SemanticError("Expression's term type incompatible with unary operator");
    }

    return additiveOperatorType;
}

TypeDescriptorPtr processTerm(TreeNodePtr node) {
    if(node->category != TERM_NODE) {
        UnexpectedNodeCategoryError(TERM_NODE, node->category);
    }

    TreeNodePtr factorNode = node->subtrees[0];
    TreeNodePtr multiplicativeOperatorNode = node->subtrees[1];
    TreeNodePtr termNode = node->subtrees[2];

    if(multiplicativeOperatorNode == NULL) {
        return processFactor(factorNode);
    }

    TypeDescriptorPtr factorType = processFactor(factorNode);
    TypeDescriptorPtr termType = processTerm(termNode);
    TypeDescriptorPtr multiplicativeOpType = processMultiplicativeOperator(multiplicativeOperatorNode);

    if(!equivalentTypes(factorType, multiplicativeOpType) ||
       !equivalentTypes(termType, multiplicativeOpType)) {
        SemanticError("Term's factors of incompatible types");
    }

    return factorType;
}

TypeDescriptorPtr processFactor(TreeNodePtr node) {
    if(node->category != FACTOR_NODE) {
        UnexpectedNodeCategoryError(FACTOR_NODE, node->category);
    }

    TreeNodePtr specificFactorNode = node->subtrees[0];
    switch (specificFactorNode->category) {
        case VALUE_NODE: {
            return processValueFactor(specificFactorNode);
        }
        case INTEGER_NODE: {
            return processIntegerFactor(specificFactorNode);
        }
        case FUNCTION_CALL_NODE: {
            return processFunctionCallFactor(specificFactorNode);
        }
        case EXPRESSION_NODE:
            return processExpression(specificFactorNode);
        default:
            UnexpectedChildNodeCategoryError(FACTOR_NODE, specificFactorNode->category);
    }
}

TypeDescriptorPtr processValueFactor(TreeNodePtr node) {
    Value value = processValue(node);
    switch (value.category) {
        case ARRAY_VALUE:
        case ARRAY_REFERENCE:
            if(value.type->size == 1) {
                addCommand("      CONT");
            } else {
                addCommand("      LDMV   %d", value.type->size);
            }
            break;
        case REFERENCE:
            addCommand("      LVLI   %d, %d", value.level, value.content.displacement);
            break;
        case VALUE:
            addCommand("      LDVL   %d,%d", value.level, value.content.displacement);
            break;
        case CONSTANT: {
            addCommand("      LDCT   %d", value.content.value);
        }
            break;
    }
    return value.type;
}

TypeDescriptorPtr processIntegerFactor(TreeNodePtr node) {
    int integer = processInteger(node);
    addCommand("      LDCT   %d", integer);
    return getSymbolTable()->integerTypeDescriptor;
}

TypeDescriptorPtr processFunctionCallFactor(TreeNodePtr node) {
    TypeDescriptorPtr functionReturnType = processFunctionCall(node);
    if (functionReturnType == NULL) {
        SemanticError("Void function used in expression");
    }
    return functionReturnType;
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
void LabelAlreadyDefinedError(char* identifier) {
    char message[255];
    sprintf(message, "Can't define label %s again", identifier);
    SemanticError(message);
}

void UndeclaredLabelError(char* identifier) {
    char message[255];
    sprintf(message, "Undeclared label %s", identifier);
    SemanticError(message);
}

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
void addCommand(const char* commandFormat, ...) {
    va_list args;
    va_start(args, commandFormat);
    vprintf(commandFormat, args);
    printf("\n");
    va_end(args);
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