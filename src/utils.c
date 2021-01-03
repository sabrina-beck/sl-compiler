#include "utils.h"

#include <stdio.h>
#include <string.h>

#define FUNCTION_PARAMETERS_DISPLACEMENT -5;

bool equivalentParameters(ParameterDescriptorsListPtr params1, ParameterDescriptorsListPtr params2);

/** Symbol table auxiliary functions **/
void initializeSymbolTable();
void addSymbolTableEntry(SymbolTableEntryPtr entry);

TypeDescriptorPtr newPredefinedTypeDescriptor(int size, PredefinedType predefinedType);
SymbolTableEntryPtr newConstant(int level, char* identifier, int value, TypeDescriptorPtr typeDescriptor);
SymbolTableEntryPtr newPseudoFunction(int level, char* identifier, PseudoFunction pseudoFunction);

/** Level counter **/
int currentFunctionLevel = 0;

/**
 * Interface functions implementation
 **/

ParameterPtr concatenateParameters(ParameterPtr parameters1, ParameterPtr parameters2) {
    if(parameters1 == NULL) {
        return parameters2;
    }

    if(parameters2 == NULL) {
        return parameters1;
    }

    ParameterPtr current = parameters1;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = parameters2;

    return parameters1;
}

int totalParametersSize(ParameterPtr parameter) {
    int size = 0;
    ParameterPtr current = parameter;
    while (current != NULL) {
        if(current->passage == VARIABLE_PARAMETER) {
            size += 1;
        } else {
            size += current->type->size;
        }
        current = current->next;
    }
    return size;
}

/**
 * Symbol Table
 **/
SymbolTablePtr symbolTable = NULL;
SymbolTablePtr getSymbolTable() {
    if(symbolTable == NULL) {
        initializeSymbolTable();
    }
    return symbolTable;
}

void initializeSymbolTable() {

    /* predefined types descriptors */
    TypeDescriptorPtr integerTypeDescriptor = newPredefinedTypeDescriptor(1, INTEGER);
    TypeDescriptorPtr booleanTypeDescriptor = newPredefinedTypeDescriptor(1, BOOLEAN);

    symbolTable = malloc(sizeof(SymbolTablePtr));
    symbolTable->stack = newStack();
    symbolTable->integerTypeDescriptor = integerTypeDescriptor;
    symbolTable->booleanTypeDescriptor = booleanTypeDescriptor;

    /* predefined types */
    addType("integer", integerTypeDescriptor);
    addType("boolean", booleanTypeDescriptor);

    /* predefined constants */
    SymbolTableEntryPtr falseEntry = newConstant(0, "false", 0, booleanTypeDescriptor);
    push(symbolTable->stack, falseEntry);

    SymbolTableEntryPtr trueEntry = newConstant(0, "true", 1, booleanTypeDescriptor);
    push(symbolTable->stack, trueEntry);

    /* pseudo functions */
    SymbolTableEntryPtr readFunction = newPseudoFunction(0, "read", READ);
    push(symbolTable->stack, readFunction);

    SymbolTableEntryPtr writeFunction = newPseudoFunction(0, "write", WRITE);
    push(symbolTable->stack, writeFunction);
}

bool byIdentifierPredicate(void* data, void* secondParam) {
    SymbolTableEntryPtr entry = (SymbolTableEntryPtr) data;
    char* identifier = (char*) secondParam;
    if(strcmp(identifier, entry->identifier) == 0) {
        return true;
    }
    return false;
}

SymbolTableEntryPtr findIdentifier(char* identifier) {
    return (SymbolTableEntryPtr) find(getSymbolTable()->stack, identifier, byIdentifierPredicate);
}

bool byLastFunctionInLevel(void* data, void* secondParam) {
    SymbolTableEntryPtr entry = (SymbolTableEntryPtr) data;
    int* level = (int*) secondParam;
    return entry->category == FUNCTION_SYMBOL && entry->level == *level;
}

FunctionDescriptorPtr findCurrentFunctionDescriptor() {
    if(currentFunctionLevel == 0) {
        return getSymbolTable()->mainFunctionDescriptor;
    }

    SymbolTableEntryPtr entry = (SymbolTableEntryPtr) find(getSymbolTable()->stack, &currentFunctionLevel, byLastFunctionInLevel);

    if(entry == NULL || entry->category != FUNCTION_SYMBOL) {
        fprintf(stderr, "Expected current function descriptor\n");
        exit(0);
    }

    return entry->description.functionDescriptor;
}

ParameterDescriptorPtr newParameterDescriptor(ParameterPtr parameter, int displacement) {
    ParameterDescriptorPtr parameterDescriptor = malloc(sizeof(ParameterDescriptorPtr));
    parameterDescriptor->displacement = displacement;
    parameterDescriptor->type = parameter->type;
    parameterDescriptor->parameterPassage = parameter->passage;

    return parameterDescriptor;
}

ParameterDescriptorsListPtr newParameterDescriptorsRec(ParameterPtr parameter, int* displacement) {
    if(parameter == NULL) {
        return NULL;
    }

    if(parameter->passage == VARIABLE_PARAMETER) {
        *displacement -= 1;
    } else {
        *displacement -= parameter->type->size;
    }

    ParameterDescriptorsListPtr parameterDescriptor = malloc(sizeof(ParameterDescriptorsList));
    parameterDescriptor->descriptor = newParameterDescriptor(parameter, *displacement);

    ParameterDescriptorsListPtr nextParameters = newParameterDescriptorsRec(parameter->next, displacement);
    parameterDescriptor->next = nextParameters;

    return parameterDescriptor;
}

ParameterDescriptorsListPtr inverseParametersDescriptors(ParameterDescriptorsListPtr parameters) {
    ParameterDescriptorsListPtr current = parameters;
    ParameterDescriptorsListPtr previous = NULL;
    ParameterDescriptorsListPtr next = NULL;

    while(current != NULL) {
        next = current->next;

        current->next = previous;

        previous = current;
        current = next;
    }

    if(previous != NULL) {
        return previous;
    }

    return current;
}

ParameterDescriptorsListPtr newParameterDescriptors(ParameterPtr parameters) {
    int displacement = 1 + FUNCTION_PARAMETERS_DISPLACEMENT;

    return newParameterDescriptorsRec(parameters, &displacement);
}

TypeDescriptorPtr newFunctionType(FunctionHeaderPtr functionHeader) {
    FunctionTypeDescriptorPtr functionTypeDescriptor = malloc(sizeof(FunctionTypeDescriptor));
    functionTypeDescriptor->parameters = inverseParametersDescriptors(newParameterDescriptors(functionHeader->parameters));
    functionTypeDescriptor->returnType = functionHeader->returnType;

    TypeDescriptorPtr type = malloc(sizeof(TypeDescriptorPtr));
    type->category = FUNCTION_TYPE;
    type->size = 3; // generalized address
    type->description.functionTypeDescriptor = functionTypeDescriptor;

    return type;
}

TypeDescriptorPtr newArrayType(int dimension, TypeDescriptorPtr elementType) {
    ArrayDescriptorPtr arrayDescriptor = malloc(sizeof(ArrayDescriptor));
    arrayDescriptor->dimension = dimension;
    arrayDescriptor->elementType = elementType;

    TypeDescriptorPtr typeDescriptor = malloc(sizeof(TypeDescriptorPtr));
    typeDescriptor->category = ARRAY_TYPE;
    typeDescriptor->size = dimension * elementType->size;;
    typeDescriptor->description.arrayDescriptor = arrayDescriptor;

    return typeDescriptor;

}

FunctionDescriptorPtr addMainFunction() {

    FunctionDescriptorPtr functionDescriptor = malloc(sizeof(FunctionDescriptor));

    functionDescriptor->variablesDisplacement = 0;
    functionDescriptor->headerMepaLabel = -1; // main can't be invoked
    functionDescriptor->returnMepaLabel = -1;
    functionDescriptor->bodyMepaLabel = -1;
    functionDescriptor->parametersSize = 0;
    functionDescriptor->parameters = NULL; // main has no parameters
    functionDescriptor->returnDisplacement = -1; // main has no return
    functionDescriptor->returnType = NULL; // VOID

    getSymbolTable()->mainFunctionDescriptor = functionDescriptor;

    return functionDescriptor;
}

void addParameter(char* identifier, ParameterDescriptorPtr parameterDescriptor) {
    SymbolTableEntryPtr symbol = malloc(sizeof(SymbolTableEntry));
    symbol->category = PARAMETER_SYMBOL;
    symbol->level = currentFunctionLevel;
    symbol->identifier = identifier;
    symbol->description.parameterDescriptor = parameterDescriptor;

    addSymbolTableEntry(symbol);
}

ParameterDescriptorsListPtr addParameterEntries(ParameterPtr parameters) {
    ParameterDescriptorsListPtr parameterDescriptors = newParameterDescriptors(parameters);

    ParameterDescriptorsListPtr currentDescriptor = parameterDescriptors;
    ParameterPtr currentParameter = parameters;
    while(currentDescriptor != NULL && currentParameter != NULL) {
        addParameter(currentParameter->name, currentDescriptor->descriptor);

        currentDescriptor = currentDescriptor->next;
        currentParameter = currentParameter->next;
    }

    return parameterDescriptors;
}

SymbolTableEntryPtr addFunction(FunctionHeaderPtr functionHeader) {
    currentFunctionLevel++;

    FunctionDescriptorPtr functionDescriptor = malloc(sizeof(FunctionDescriptor));

    functionDescriptor->headerMepaLabel = nextMEPALabel();
    functionDescriptor->returnMepaLabel = nextMEPALabel();
    functionDescriptor->bodyMepaLabel = -1;
    functionDescriptor->variablesDisplacement = 0;
    functionDescriptor->parametersSize = totalParametersSize(functionHeader->parameters);
    functionDescriptor->returnType = functionHeader->returnType;
    functionDescriptor->parameters = inverseParametersDescriptors(addParameterEntries(functionHeader->parameters));
    functionDescriptor->functionType = newFunctionType(functionHeader);

    if(functionHeader->returnType != NULL) {
        if(functionHeader->parameters == NULL) {
            functionDescriptor->returnDisplacement =  - functionHeader->returnType->size + 1 + FUNCTION_PARAMETERS_DISPLACEMENT;
        } else {
            functionDescriptor->returnDisplacement =
                    - functionHeader->returnType->size - functionDescriptor->parametersSize + 1 + FUNCTION_PARAMETERS_DISPLACEMENT;
        }
    }

    SymbolTableEntryPtr symbol = malloc(sizeof(SymbolTableEntry));
    symbol->category = FUNCTION_SYMBOL;
    symbol->level = currentFunctionLevel;
    symbol->identifier = functionHeader->name;
    symbol->description.functionDescriptor = functionDescriptor;

    addSymbolTableEntry(symbol);

    return symbol;
}

void addLabel(char* identifier) {
    LabelDescriptorPtr labelDescriptor = malloc(sizeof(LabelDescriptor));
    labelDescriptor->mepaLabel = nextMEPALabel();
    labelDescriptor->defined = false;

    SymbolTableEntryPtr symbol = malloc(sizeof(SymbolTableEntry));
    symbol->category = LABEL_SYMBOL;
    symbol->level = currentFunctionLevel;
    symbol->identifier = identifier;
    symbol->description.labelDescriptor = labelDescriptor;

    addSymbolTableEntry(symbol);
}

void addType(char* identifier, TypeDescriptorPtr typeDescriptor) {
    SymbolTableEntryPtr symbol = malloc(sizeof(SymbolTableEntry));
    symbol->category = TYPE_SYMBOL;
    symbol->level = currentFunctionLevel;
    symbol->identifier = identifier;
    symbol->description.typeDescriptor = typeDescriptor;

    addSymbolTableEntry(symbol);
}

void addVariable(char* identifier, TypeDescriptorPtr typeDescriptor) {
    FunctionDescriptorPtr functionDescriptor = findCurrentFunctionDescriptor(getSymbolTable());
    if(functionDescriptor == NULL) {
        fprintf(stderr, "addVariable: Expected current function descriptor\n");
        exit(0);
    }

    VariableDescriptorPtr variableDescriptor = malloc(sizeof(VariableDescriptor));

    variableDescriptor->displacement = functionDescriptor->variablesDisplacement;
    functionDescriptor->variablesDisplacement += typeDescriptor->size;

    variableDescriptor->type = typeDescriptor;

    SymbolTableEntryPtr symbol = malloc(sizeof(SymbolTableEntry));
    symbol->category = VARIABLE_SYMBOL;
    symbol->level = currentFunctionLevel;
    symbol->identifier = identifier;
    symbol->description.variableDescriptor = variableDescriptor;

    addSymbolTableEntry(symbol);
}

void addSymbolTableEntry(SymbolTableEntryPtr entry) {
    push(getSymbolTable()->stack, entry);
}

int getFunctionLevel() {
    return currentFunctionLevel;
}

void endFunctionLevel() {
    SymbolTablePtr symbolTablePtr = getSymbolTable();
    currentFunctionLevel--;

    Stack* auxStack = newStack();

    SymbolTableEntryPtr lastPoped = pop(symbolTablePtr->stack);
    while (lastPoped != NULL && lastPoped->level > currentFunctionLevel) {
        if (lastPoped->category == FUNCTION_SYMBOL) {
            if(lastPoped->level == currentFunctionLevel + 1) {
                push(auxStack, lastPoped);
            }
        }
        lastPoped = pop(symbolTablePtr->stack);
    }
    push(symbolTablePtr->stack, lastPoped);

    // pushing back the functions at level+1 that are still accessible
    while (auxStack->top != NULL) {
        SymbolTableEntryPtr poped = pop(auxStack);
        push(symbolTablePtr->stack, poped);
    }
    free(auxStack);
}

bool equivalentTypes(TypeDescriptorPtr type1, TypeDescriptorPtr type2) {

    if(type1 == NULL || type2 == NULL) {
        return true;
    }

    switch (type1->category) {
        case PREDEFINED_TYPE:
            return type1->category == type2->category &&
                   type1->size == type2->size &&
                   type1->description.predefinedType == type2->description.predefinedType;
        case ARRAY_TYPE:
            return type1->category == type2->category &&
                   type1->size == type2->size &&
                   type1->description.arrayDescriptor->dimension == type2->description.arrayDescriptor->dimension &&
                   equivalentTypes(type1->description.arrayDescriptor->elementType, type2->description.arrayDescriptor->elementType);
        case FUNCTION_TYPE: {
            ParameterDescriptorsListPtr params1 = type1->description.functionTypeDescriptor->parameters;
            ParameterDescriptorsListPtr params2 = type2->description.functionTypeDescriptor->parameters;

            return type1->category == type2->category &&
                   type1->size == type2->size &&
                   equivalentTypes(type1->description.functionTypeDescriptor->returnType,
                                   type2->description.functionTypeDescriptor->returnType) &&
                   equivalentParameters(params1, params2);
        }
    }
}

bool equivalentFunctions(TypeDescriptorPtr functionType, FunctionDescriptorPtr functionDescriptor) {
    FunctionTypeDescriptorPtr function1Descriptor = functionType->description.functionTypeDescriptor;
    return equivalentTypes(function1Descriptor->returnType, functionDescriptor->returnType)
            && equivalentParameters(function1Descriptor->parameters, functionDescriptor->parameters);
}

bool equivalentParameters(ParameterDescriptorsListPtr params1, ParameterDescriptorsListPtr params2) {
    ParameterDescriptorsListPtr currentParam1 = params1;
    ParameterDescriptorsListPtr currentParam2 = params2;
    while (currentParam1 != NULL && currentParam2 != NULL) {

        TypeDescriptorPtr param1Type = currentParam1->descriptor->type;
        TypeDescriptorPtr param2Type = currentParam2->descriptor->type;

        if (!equivalentTypes(param1Type, param2Type)) {
            return false;
        }

        currentParam1 = currentParam1->next;
        currentParam2 = currentParam2->next;
    }

    if(currentParam1 != NULL || currentParam2 != NULL) {
        return false;
    }

    return true;
}

/** Mepa Label generator auxiliary structures **/
int mepaLabelCounter = 0;
int nextMEPALabel(){
    return ++mepaLabelCounter;
}

/**
 * Private auxiliary functions
 **/

TypeDescriptorPtr newPredefinedTypeDescriptor(int size, PredefinedType predefinedType) {
    TypeDescriptorPtr predefinedTypeDescriptor = malloc(sizeof(TypeDescriptor));
    predefinedTypeDescriptor->category = PREDEFINED_TYPE;
    predefinedTypeDescriptor->size = size;
    predefinedTypeDescriptor->description.predefinedType = predefinedType;
    return predefinedTypeDescriptor;
}

SymbolTableEntryPtr newConstant(int level, char* identifier, int value, TypeDescriptorPtr typeDescriptor) {
    ConstantDescriptorPtr constantDescriptor = malloc(sizeof(ConstantDescriptor));
    constantDescriptor->value = value;
    constantDescriptor->type = typeDescriptor;

    SymbolTableEntryPtr symbol = malloc(sizeof(SymbolTableEntry));
    symbol->category = CONSTANT_SYMBOL;
    symbol->level = level;
    symbol->identifier = identifier;
    symbol->description.constantDescriptor = constantDescriptor;

    return symbol;
}

SymbolTableEntryPtr newPseudoFunction(int level, char* identifier, PseudoFunction pseudoFunction) {
    SymbolTableEntryPtr symbol = malloc(sizeof(SymbolTableEntry));
    symbol->category = PSEUDO_FUNCTION_SYMBOL;
    symbol->level = level;
    symbol->identifier = identifier;
    symbol->description.pseudoFunction = pseudoFunction;

    return symbol;
}

/****
 * Others
 ****/
Value variableSymbolToValue(SymbolTableEntryPtr entry) {
    Value value;
    value.type = entry->description.variableDescriptor->type;
    value.level = entry->level;
    value.content.displacement = entry->description.variableDescriptor->displacement;

    switch(entry->description.variableDescriptor->type->category) {
        case PREDEFINED_TYPE:
            value.category = VALUE;
            break;
        case ARRAY_TYPE:
            value.category = ARRAY_VALUE;
            break;
        default: {
            fprintf(stderr, "Expected predefined type or array type to convert to Value\n");
            exit(0);
        }
    }

    return value;
}

Value parameterSymbolToValue(SymbolTableEntryPtr entry) {
    Value value;
    value.type = entry->description.parameterDescriptor->type;
    value.level = entry->level;
    value.content.displacement = entry->description.parameterDescriptor->displacement;

    ParameterDescriptorPtr parameterDescriptor = entry->description.parameterDescriptor;
    switch(parameterDescriptor->type->category) {
        case PREDEFINED_TYPE:
            if(parameterDescriptor->parameterPassage == VARIABLE_PARAMETER) {
                value.category = REFERENCE;
            } else {
                value.category = VALUE;
            }
            break;
        case ARRAY_TYPE:
            if(parameterDescriptor->parameterPassage == VARIABLE_PARAMETER) {
                value.category = ARRAY_REFERENCE;
            } else {
                value.category = ARRAY_VALUE;
            }
            break;
        default: {
            fprintf(stderr, "Expected predefined type or array type to convert to Value\n");
            exit(0);
        }
    }
    return value;
}

Value constantSymbolToValue(SymbolTableEntryPtr entry) {
    Value value;
    value.type = entry->description.constantDescriptor->type;
    value.level = entry->level;
    value.content.value = entry->description.constantDescriptor->value;
    value.category = CONSTANT;
    return value;
}

Value valueFromEntry(SymbolTableEntryPtr entry) {
    switch(entry->category) {
        case VARIABLE_SYMBOL: {
            return variableSymbolToValue(entry);
        }
        case PARAMETER_SYMBOL: {
            return parameterSymbolToValue(entry);
        }
        case CONSTANT_SYMBOL: {
            return constantSymbolToValue(entry);
        }
        default: {
            fprintf(stderr, "Unexpected entry category to convert to Value\n");
            exit(0);
        }
    }
}

/****
 * Debug
 ****/
char* getSymbolTableCategoryName(SymbolTableCategory category) {
    switch (category) {
        case TYPE_SYMBOL:
            return "TYPE_SYMBOL";
        case CONSTANT_SYMBOL:
            return "CONSTANT_SYMBOL";
        case VARIABLE_SYMBOL:
            return "VARIABLE_SYMBOL";
        case PARAMETER_SYMBOL:
            return "PARAMETER_SYMBOL";
        case PSEUDO_FUNCTION_SYMBOL:
            return "PSEUDO_FUNCTION_SYMBOL";
        case FUNCTION_SYMBOL:
            return "FUNCTION_SYMBOL";
        case LABEL_SYMBOL:
            return "LABEL_SYMBOL";
    }
}

char* getTypeCategoryName(TypeCategory category) {
    switch (category) {
        case PREDEFINED_TYPE:
            return "PREDEFINED_TYPE";
        case ARRAY_TYPE:
            return "ARRAY_TYPE";
        case FUNCTION_TYPE:
            return "FUNCTION_TYPE";
    }
}

char* getPredefinedTypeName(PredefinedType category) {
    switch(category) {
        case INTEGER:
            return "INTEGER";
        case BOOLEAN:
            return "BOOLEAN";
    }
}