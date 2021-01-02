#include "utils.h"

#include <stdio.h>
#include <string.h>

#define FUNCTION_PARAMETERS_DISPLACEMENT -5;

bool equivalentParameters(List* params1, List* params2);

/** Symbol table auxiliary functions **/
TypeDescriptorPtr newPredefinedTypeDescriptor(int size, PredefinedType predefinedType);
SymbolTableEntryPtr newConstant(int level, char* identifier, int value, TypeDescriptorPtr typeDescriptor);
SymbolTableEntryPtr newPseudoFunction(int level, char* identifier, PseudoFunction pseudoFunction);

/** Mepa Label generator auxiliary structures **/
int mepaLabelCounter = 0;
int countDigits(int number);

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
        size += current->type->size;
        current = current->next;
    }
    return size;
}

SymbolTablePtr initializeSymbolTable() {

    /* predefined types descriptors */
    TypeDescriptorPtr integerTypeDescriptor = newPredefinedTypeDescriptor(1, INTEGER);
    TypeDescriptorPtr booleanTypeDescriptor = newPredefinedTypeDescriptor(1, BOOLEAN);

    SymbolTablePtr symbolTable = malloc(sizeof(SymbolTablePtr));
    symbolTable->stack = newStack();
    symbolTable->integerTypeDescriptor = integerTypeDescriptor;
    symbolTable->booleanTypeDescriptor = booleanTypeDescriptor;

    /* predefined types */
    addType(symbolTable, "integer", integerTypeDescriptor);
    addType(symbolTable, "boolean", booleanTypeDescriptor);

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

    return symbolTable;
}

bool byIdentifierPredicate(void* data, void* secondParam) {
    SymbolTableEntryPtr entry = (SymbolTableEntryPtr) data;
    char* identifier = (char*) secondParam;
    if(strcmp(identifier, entry->identifier) == 0) {
        return true;
    }
    return false;
}

SymbolTableEntryPtr findIdentifier(SymbolTablePtr symbolTable, char* identifier) {
    return (SymbolTableEntryPtr) find(symbolTable->stack, identifier, byIdentifierPredicate);
}

bool byLastFunctionInLevel(void* data, void* secondParam) {
    SymbolTableEntryPtr entry = (SymbolTableEntryPtr) data;
    int* level = (int*) secondParam;
    return entry->category == FUNCTION_SYMBOL && entry->level == *level;
}

FunctionDescriptorPtr findCurrentFunctionDescriptor(SymbolTablePtr symbolTable) {
    if(currentFunctionLevel == 0) {
        return symbolTable->mainFunctionDescriptor;
    }

    SymbolTableEntryPtr entry = (SymbolTableEntryPtr) find(symbolTable->stack, &currentFunctionLevel, byLastFunctionInLevel);

    if(entry == NULL || entry->category != FUNCTION_SYMBOL) {
        fprintf(stderr, "Expected current function descriptor\n");
        exit(0);
    }

    return entry->description.functionDescriptor;
}

SymbolTableEntryPtr newParameter(ParameterPtr parameter, int displacement) {
    ParameterDescriptorPtr parameterDescriptor = malloc(sizeof(ParameterDescriptorPtr));
    parameterDescriptor->displacement = displacement;
    parameterDescriptor->type = parameter->type;
    parameterDescriptor->parameterPassage = parameter->passage;

    SymbolTableEntryPtr symbol = malloc(sizeof(SymbolTableEntry));
    symbol->category = PARAMETER_SYMBOL;
    symbol->level = currentFunctionLevel;
    symbol->identifier = parameter->name;
    symbol->description.parameterDescriptor = parameterDescriptor;

    return symbol;
}

List* newParameterEntries(ParameterPtr parameters) {
    List* list = newList();

    ParameterPtr current = parameters;
    int displacement = FUNCTION_PARAMETERS_DISPLACEMENT;
    while(current != NULL) {
        SymbolTableEntryPtr parameterEntry = newParameter(parameters, displacement);
        add(list, parameterEntry); // invert the list again, so the parameters order is correct

        displacement -= parameterEntry->description.parameterDescriptor->type->size;
        current = current->next;
    }

    return list;
}

List* addParameterEntries(SymbolTablePtr symbolTable, ParameterPtr parameters) {
    List* entries = newParameterEntries(parameters);

    LinkedNode* current = entries->front;
    while(current != NULL) {
        SymbolTableEntryPtr symbol = (SymbolTableEntryPtr) current->data;
        addSymbolTableEntry(symbolTable, symbol);

        current = current->next;
    }

    return entries;
}

TypeDescriptorPtr newFunctionType(FunctionHeaderPtr functionHeader) {
    FunctionTypeDescriptorPtr functionTypeDescriptor = malloc(sizeof(FunctionTypeDescriptor));
    functionTypeDescriptor->params = newParameterEntries(functionHeader->parameters);
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

    return typeDescriptor;

}

void addMainFunction(SymbolTablePtr symbolTable) {

    FunctionDescriptorPtr functionDescriptor = malloc(sizeof(FunctionDescriptor));

    functionDescriptor->variablesDisplacement = 0;
    functionDescriptor->mepaLabel = NULL; // main can't be invoked
    functionDescriptor->returnLabel = NULL; //FIXME return in the main function
    functionDescriptor->parametersSize = 0;
    functionDescriptor->params = NULL; // main has no parameters
    functionDescriptor->returnDisplacement = -1; // main has no return
    functionDescriptor->returnType = NULL; // VOID

    symbolTable->mainFunctionDescriptor = functionDescriptor;
}

SymbolTableEntryPtr addFunction(SymbolTablePtr symbolTable, FunctionHeaderPtr functionHeader) {

    FunctionDescriptorPtr functionDescriptor = malloc(sizeof(FunctionDescriptor));

    functionDescriptor->mepaLabel = nextMEPALabel();
    functionDescriptor->returnLabel = nextMEPALabel();
    functionDescriptor->variablesDisplacement = 0;
    functionDescriptor->parametersSize = totalParametersSize(functionHeader->parameters);
    functionDescriptor->returnType = functionHeader->returnType;
    functionDescriptor->params = addParameterEntries(symbolTable, functionHeader->parameters);

    if(functionHeader->parameters == NULL) {
        functionDescriptor->returnDisplacement = FUNCTION_PARAMETERS_DISPLACEMENT;
    } else {
        functionDescriptor->returnDisplacement = -functionDescriptor->parametersSize + FUNCTION_PARAMETERS_DISPLACEMENT;
    }

    SymbolTableEntryPtr symbol = malloc(sizeof(SymbolTableEntry));
    symbol->category = FUNCTION_SYMBOL;
    symbol->level = ++currentFunctionLevel;
    symbol->identifier = functionHeader->name;
    symbol->description.functionDescriptor = functionDescriptor;
    symbol->description.typeDescriptor = newFunctionType(functionHeader);

    addSymbolTableEntry(symbolTable, symbol);

    return symbol;
}

void addLabel(SymbolTablePtr symbolTable, char* identifier) {
    LabelDescriptorPtr labelDescriptor = malloc(sizeof(LabelDescriptor));
    labelDescriptor->mepaLabel = nextMEPALabel();
    labelDescriptor->defined = false;

    SymbolTableEntryPtr symbol = malloc(sizeof(SymbolTableEntry));
    symbol->category = LABEL_SYMBOL;
    symbol->level = currentFunctionLevel;
    symbol->identifier = identifier;
    symbol->description.labelDescriptor = labelDescriptor;

    addSymbolTableEntry(symbolTable, symbol);
}

void addType(SymbolTablePtr symbolTable, char* identifier, TypeDescriptorPtr typeDescriptor) {
    SymbolTableEntryPtr symbol = malloc(sizeof(SymbolTableEntry));
    symbol->category = TYPE_SYMBOL;
    symbol->level = currentFunctionLevel;
    symbol->identifier = identifier;
    symbol->description.typeDescriptor = typeDescriptor;

    addSymbolTableEntry(symbolTable, symbol);
}

void addVariable(SymbolTablePtr symbolTable, char* identifier, TypeDescriptorPtr typeDescriptor) {
    FunctionDescriptorPtr functionDescriptor = findCurrentFunctionDescriptor(symbolTable);
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

    addSymbolTableEntry(symbolTable, symbol);
}

void addSymbolTableEntry(SymbolTablePtr symbolTable, SymbolTableEntryPtr entry) {
    push(symbolTable->stack, entry);

    if(entry->category == FUNCTION_SYMBOL) {
        LinkedNode* current = entry->description.functionDescriptor->params->front;

        while (current != NULL) {
            SymbolTableEntryPtr paramEntry = (SymbolTableEntryPtr) current->data;
            addSymbolTableEntry(symbolTable, paramEntry);
            current = current->next;
        }
    }
}

int getFunctionLevel() {
    return currentFunctionLevel;
}

void endFunctionLevel(SymbolTablePtr symbolTablePtr) {
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
            List* params1 = type1->description.functionTypeDescriptor->params;
            List* params2 = type2->description.functionTypeDescriptor->params;

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
            && equivalentParameters(function1Descriptor->params, functionDescriptor->params);
}

bool equivalentParameters(List* params1, List* params2) {
    if (params1->size != params2->size) {
        return false;
    }

    LinkedNode* currentParam1 = params1->front;
    LinkedNode* currentParam2 = params2->front;
    while (currentParam1 != NULL && currentParam2 != NULL) {
        SymbolTableEntryPtr param1 = (SymbolTableEntryPtr) currentParam1->data;
        TypeDescriptorPtr param1Type = param1->description.parameterDescriptor->type;

        SymbolTableEntryPtr param2 = (SymbolTableEntryPtr) currentParam2->data;
        TypeDescriptorPtr param2Type = param2->description.parameterDescriptor->type;

        if (!equivalentTypes(param1Type, param2Type)) {
            return false;
        }

        currentParam1 = currentParam1->next;
        currentParam2 = currentParam2->next;
    }

    return true;
}

char* nextMEPALabel(){
    mepaLabelCounter++;

    int digitsCount = countDigits(mepaLabelCounter);
    char* mepaLabel = malloc(sizeof(char)*(digitsCount+1)); // TODO free this (memory leak!)
    sprintf(mepaLabel, "L%d", mepaLabelCounter);

    return mepaLabel;
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

int countDigits(int number) {
    int count = 0;

    do {
        count++;
        number /= 10;
    } while (number != 0);

    return count;
}

/****
 * Others
 ****/
Value variableSymbolToValue(SymbolTableEntryPtr entry) {
    Value value;
    value.type = entry->description.variableDescriptor->type;
    value.level = entry->level;
    value.content.displacement = entry->description.variableDescriptor->displacement;
    value.category = VALUE;
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