#include "symboltable.h"

#include <stdio.h>
#include <string.h>

#define FUNCTION_PARAMETERS_DISPLACEMENT -5;

/** Auxiliary structures private functions **/
int totalParametersSize(ParameterPtr parameter);

/** Level counter **/
int currentFunctionLevel = 0;

/***********************************************************************************************************************
 * Symbol Table
 **********************************************************************************************************************/
/* Private Declarations */
void initializeSymbolTable();
void addSymbolTableEntry(SymbolTableEntryPtr entry);

ParameterDescriptorsListPtr newParameterDescriptors(ParameterPtr parameter);
ParameterDescriptorsListPtr addParameterEntries(ParameterPtr parameters);

/* Implementation */

SymbolTablePtr symbolTable = NULL;
SymbolTablePtr getSymbolTable() {
    if(symbolTable == NULL) {
        initializeSymbolTable();
    }
    return symbolTable;
}

/*
 * The accessible symbol for an identifier is the last one pushed on the stack (symbol table) with this identifier
 */
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

/*
 * The function being currently compiled is the last function entry in the symbol table
 */
bool byLastFunctionInLevel(void* data, void* secondParam) {
    SymbolTableEntryPtr entry = (SymbolTableEntryPtr) data;
    int* level = (int*) secondParam;
    return entry->category == FUNCTION_SYMBOL && entry->level == *level;
}
FunctionDescriptorPtr findCurrentFunctionDescriptor() {
    // Level 0 is the main function and the main function is not on the stack since its identifier is not accessible
    if(currentFunctionLevel == 0) {
        return getSymbolTable()->mainFunctionDescriptor;
    }

    SymbolTableEntryPtr entry = (SymbolTableEntryPtr) find(getSymbolTable()->stack, &currentFunctionLevel, byLastFunctionInLevel);

    if(entry == NULL || entry->category != FUNCTION_SYMBOL) {
        fprintf(stderr, "Expected current function descriptor but got a %s\n", getSymbolTableCategoryName(entry->category));
        exit(0);
    }

    return entry->description.functionDescriptor;
}


TypeDescriptorPtr newFunctionType(FunctionHeaderPtr functionHeader) {
    FunctionTypeDescriptorPtr functionTypeDescriptor = malloc(sizeof(FunctionTypeDescriptor));
    functionTypeDescriptor->parameters = newParameterDescriptors(functionHeader->parameters);
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
    typeDescriptor->size = dimension * elementType->size;
    typeDescriptor->description.arrayDescriptor = arrayDescriptor;

    return typeDescriptor;
}


SymbolTableEntryPtr addFunction(FunctionHeaderPtr functionHeader) {
    // Since it is a new function being compiled, the level increased
    currentFunctionLevel++;

    FunctionDescriptorPtr functionDescriptor = malloc(sizeof(FunctionDescriptor));

    functionDescriptor->headerMepaLabel = nextMEPALabel();
    functionDescriptor->returnMepaLabel = nextMEPALabel();
    functionDescriptor->bodyMepaLabel = -1; // this label is only created if needed
    functionDescriptor->variablesDisplacement = 0;
    functionDescriptor->parametersSize = totalParametersSize(functionHeader->parameters);
    functionDescriptor->returnType = functionHeader->returnType;
    functionDescriptor->parameters = addParameterEntries(functionHeader->parameters);
    functionDescriptor->functionType = newFunctionType(functionHeader);

    // updates return displacement only if the function has a return type
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

FunctionDescriptorPtr addMainFunction() {

    FunctionDescriptorPtr functionDescriptor = malloc(sizeof(FunctionDescriptor));

    functionDescriptor->variablesDisplacement = 0;
    functionDescriptor->headerMepaLabel = -1; // main can't be invoked
    functionDescriptor->returnMepaLabel = -1; // main can't have a return
    functionDescriptor->bodyMepaLabel = -1; // this label is only created if needed
    functionDescriptor->parameters = NULL; // main has no parameters
    functionDescriptor->parametersSize = 0;
    functionDescriptor->returnType = NULL; // main has no return, it must be VOID
    functionDescriptor->returnDisplacement = -1;

    getSymbolTable()->mainFunctionDescriptor = functionDescriptor;

    return functionDescriptor;
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
    FunctionDescriptorPtr functionDescriptor = findCurrentFunctionDescriptor();

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

/* Private implementations */

TypeDescriptorPtr newPredefinedTypeDescriptor(int size, PredefinedType predefinedType) {

    TypeDescriptorPtr predefinedTypeDescriptor = malloc(sizeof(TypeDescriptor));
    predefinedTypeDescriptor->category = PREDEFINED_TYPE;
    predefinedTypeDescriptor->size = size;
    predefinedTypeDescriptor->description.predefinedType = predefinedType;

    return predefinedTypeDescriptor;
}

void addConstant(int level, char* identifier, int value, TypeDescriptorPtr typeDescriptor) {
    ConstantDescriptorPtr constantDescriptor = malloc(sizeof(ConstantDescriptor));
    constantDescriptor->value = value;
    constantDescriptor->type = typeDescriptor;

    SymbolTableEntryPtr symbol = malloc(sizeof(SymbolTableEntry));
    symbol->category = CONSTANT_SYMBOL;
    symbol->level = level;
    symbol->identifier = identifier;
    symbol->description.constantDescriptor = constantDescriptor;

    addSymbolTableEntry(symbol);
}

void addPseudoFunction(int level, char* identifier, PseudoFunction pseudoFunction) {
    SymbolTableEntryPtr symbol = malloc(sizeof(SymbolTableEntry));
    symbol->category = PSEUDO_FUNCTION_SYMBOL;
    symbol->level = level;
    symbol->identifier = identifier;
    symbol->description.pseudoFunction = pseudoFunction;

    addSymbolTableEntry(symbol);
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
    addConstant(0, "false", 0, booleanTypeDescriptor);
    addConstant(0, "true", 1, booleanTypeDescriptor);

    /* pseudo functions */
    addPseudoFunction(0, "read", READ);
    addPseudoFunction(0, "write", WRITE);
}

void addSymbolTableEntry(SymbolTableEntryPtr entry) {
    push(getSymbolTable()->stack, entry);
}

ParameterDescriptorPtr newParameterDescriptor(ParameterPtr parameter, int displacement) {
    ParameterDescriptorPtr parameterDescriptor = malloc(sizeof(ParameterDescriptorPtr));
    parameterDescriptor->displacement = displacement;
    parameterDescriptor->type = parameter->type;
    parameterDescriptor->parameterPassage = parameter->passage;

    return parameterDescriptor;
}

/*
 * The parameters list of type ParameterPtr comes from the compiler in the order of the parameters declaration
 */
ParameterDescriptorsListPtr newParameterDescriptorsRec(ParameterPtr parameter, int* displacement) {
    if(parameter == NULL) {
        return NULL;
    }

    // Process the parameters in inversed order to calculate the displacement correctly,
    // so the first declared parameter will be in the lowest parameter position of the stack
    // this allows pushing arguments in they declared order too
    ParameterDescriptorsListPtr nextParameters = newParameterDescriptorsRec(parameter->next, displacement);

    if(parameter->passage == VARIABLE_PARAMETER) {
        // a parameter by reference has only it's address on the stack, therefore it occupies only one position
        *displacement -= 1;
    } else {
        // a parameter by value has its whole value on the stack, therefore it's size is the same as it's type
        *displacement -= parameter->type->size;
    }

    ParameterDescriptorsListPtr parameterDescriptor = malloc(sizeof(ParameterDescriptorsList));
    parameterDescriptor->descriptor = newParameterDescriptor(parameter, *displacement);
    parameterDescriptor->next = nextParameters;

    return parameterDescriptor;
}
ParameterDescriptorsListPtr newParameterDescriptors(ParameterPtr parameters) {
    int displacement = 1 + FUNCTION_PARAMETERS_DISPLACEMENT;

    return newParameterDescriptorsRec(parameters, &displacement);
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
        addParameter(currentParameter->name, currentDescriptor->descriptor);\

        currentDescriptor = currentDescriptor->next;
        currentParameter = currentParameter->next;
    }

    return parameterDescriptors;
}

/***********************************************************************************************************************
 * Level counter
 **********************************************************************************************************************/
int getFunctionLevel() {
    return currentFunctionLevel;
}

void endFunctionLevel() {
    SymbolTablePtr symbolTablePtr = getSymbolTable();
    currentFunctionLevel--;

    Stack* auxStack = newStack();

    SymbolTableEntryPtr lastPoped = pop(symbolTablePtr->stack);
    // pop from the stack all entries above the new current level, since their identifiers are not accessible anymore
    while (lastPoped != NULL && lastPoped->level > currentFunctionLevel) {

        // functions are an exception, so we keep track of all functions declared in level above
        if (lastPoped->category == FUNCTION_SYMBOL) {
            if(lastPoped->level == currentFunctionLevel + 1) {
                push(auxStack, lastPoped); //
            }
        }

        lastPoped = pop(symbolTablePtr->stack);
    }
    push(symbolTablePtr->stack, lastPoped);

    // pushing back the functions at level+1 that will be still accessible
    while (auxStack->top != NULL) {
        SymbolTableEntryPtr poped = pop(auxStack);
        push(symbolTablePtr->stack, poped);
    }
    free(auxStack);
}

/***********************************************************************************************************************
 * MEPA label counter
 **********************************************************************************************************************/
int mepaLabelCounter = 0;
int nextMEPALabel(){
    return ++mepaLabelCounter;
}

/***********************************************************************************************************************
 * Type Compatibility
 **********************************************************************************************************************/
/* Private Declarations */
bool equivalentParameters(ParameterDescriptorsListPtr params1, ParameterDescriptorsListPtr params2);

/* Implementations */

bool equivalentTypes(TypeDescriptorPtr type1, TypeDescriptorPtr type2) {

    if(type1 == NULL || type2 == NULL) {
        return true; // the compiler askes the equivalence even if one of the types are not present
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

/* Private implementations */

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

    // a different number of parameters between function types makes them incompatible
    if(currentParam1 != NULL || currentParam2 != NULL) {
        return false;
    }

    return true;
}

/***********************************************************************************************************************
 * Auxiliary structures functions
 **********************************************************************************************************************/
/* Private declarations */

Value variableSymbolToValue(SymbolTableEntryPtr entry);
Value parameterSymbolToValue(SymbolTableEntryPtr entry);
Value constantSymbolToValue(SymbolTableEntryPtr entry);

/* Implementations */

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


/* Private Implementations */

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

/***********************************************************************************************************************
 * Debug facilities
 **********************************************************************************************************************/
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