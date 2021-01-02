#include "utils.h"

#include <stdio.h>
#include <string.h>

bool equivalentParameters(List* params1, List* params2);

/** Symbol table auxiliary functions **/
TypeDescriptorPtr newPredefinedTypeDescriptor(int size, PredefinedType predefinedType);
SymbolTableEntryPtr newConstant(int level, char* identifier, int value, TypeDescriptorPtr typeDescriptor);
SymbolTableEntryPtr newPseudoFunction(int level, char* identifier, PseudoFunction pseudoFunction);

/** Mepa Label generator auxiliary structures **/
int mepaLabelCounter = 0;
int countDigits(int number);

/**
 * Interface functions implementation
 **/

SymbolTablePtr initializeSymbolTable() {

    /* predefined types descriptors */
    TypeDescriptorPtr integerTypeDescriptor = newPredefinedTypeDescriptor(1, INTEGER);
    TypeDescriptorPtr booleanTypeDescriptor = newPredefinedTypeDescriptor(1, BOOLEAN);

    SymbolTablePtr symbolTable = malloc(sizeof(SymbolTablePtr));
    symbolTable->stack = newStack();
    symbolTable->integerTypeDescriptor = integerTypeDescriptor;
    symbolTable->booleanTypeDescriptor = booleanTypeDescriptor;

    /* predefined types */
    SymbolTableEntryPtr integerTypeEntry = newType(0, "integer", integerTypeDescriptor);
    push(symbolTable->stack, integerTypeEntry);

    SymbolTableEntryPtr booleanTypeEntry = newType(0, "boolean", booleanTypeDescriptor);
    push(symbolTable->stack, booleanTypeEntry);

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

FunctionDescriptorPtr findCurrentFunctionDescriptor(SymbolTablePtr symbolTable, int level) {
    SymbolTableEntryPtr entry = (SymbolTableEntryPtr) find(symbolTable->stack, &level, byLastFunctionInLevel);
    if(entry == NULL || entry->category != FUNCTION_SYMBOL) {
        return NULL;
    }

    return entry->description.functionDescriptor;
}

SymbolTableEntryPtr newParameter(int level, char* identifier, int displacement, TypeDescriptorPtr type, ParameterPassage parameterPassage) {
    ParameterDescriptorPtr parameterDescriptor = malloc(sizeof(ParameterDescriptorPtr));
    parameterDescriptor->displacement = displacement;
    parameterDescriptor->type = type;
    parameterDescriptor->parameterPassage = parameterPassage;

    SymbolTableEntryPtr symbol = malloc(sizeof(SymbolTableEntry));
    symbol->category = PARAMETER_SYMBOL;
    symbol->level = level;
    symbol->identifier = identifier;
    symbol->description.parameterDescriptor = parameterDescriptor;

    return symbol;
}

SymbolTableEntryPtr newFunctionParameter(SymbolTableEntryPtr functionEntry, int displacement) {
    if (functionEntry->category != FUNCTION_SYMBOL) {
        fprintf(stderr, "Expected function entry for new function parameter!");
        exit(0);
    }

    FunctionDescriptorPtr functionDescriptor = functionEntry->description.functionDescriptor;

    FunctionTypeDescriptorPtr functionTypeDescriptor = malloc(sizeof(FunctionTypeDescriptor));
    functionTypeDescriptor->params = functionDescriptor->params;
    functionTypeDescriptor->returnType = functionDescriptor->returnType;

    TypeDescriptorPtr type = malloc(sizeof(TypeDescriptorPtr));
    type->category = FUNCTION_TYPE;
    type->size = 3; // generalized address
    type->description.functionTypeDescriptor = functionTypeDescriptor;

    return newParameter(functionEntry->level, functionEntry->identifier, displacement, type, FUNCTION_PARAMETER);
}

SymbolTableEntryPtr newFunctionDescriptor(int level, char* identifier, TypeDescriptorPtr returnType, List* paramEntries) {
    FunctionDescriptorPtr functionDescriptor = malloc(sizeof(FunctionDescriptor));
    functionDescriptor->mepaLabel = nextMEPALabel();
    functionDescriptor->returnLabel = nextMEPALabel();
    if(paramEntries == NULL) {
        functionDescriptor->returnDisplacement = -5;
    } else {
        functionDescriptor->returnDisplacement = -paramEntries->size - 5; // FIXME explain
    }
    functionDescriptor->returnType = returnType;
    functionDescriptor->params = paramEntries;

    SymbolTableEntryPtr symbol = malloc(sizeof(SymbolTableEntry));
    symbol->category = FUNCTION_SYMBOL;
    symbol->level = level;
    symbol->identifier = identifier;
    symbol->description.functionDescriptor = functionDescriptor;

    return symbol;
}

SymbolTableEntryPtr newLabel(int level, char* identifier) {
    LabelDescriptorPtr labelDescriptor = malloc(sizeof(LabelDescriptor));
    labelDescriptor->mepaLabel = nextMEPALabel();
    labelDescriptor->defined = false;

    SymbolTableEntryPtr symbol = malloc(sizeof(SymbolTableEntry));
    symbol->category = LABEL_SYMBOL;
    symbol->level = level;
    symbol->identifier = identifier;
    symbol->description.labelDescriptor = labelDescriptor;

    return symbol;
}


SymbolTableEntryPtr newType(int level, char* identifier, TypeDescriptorPtr typeDescriptor) {
    SymbolTableEntryPtr symbol = malloc(sizeof(SymbolTableEntry));
    symbol->category = TYPE_SYMBOL;
    symbol->level = level;
    symbol->identifier = identifier;
    symbol->description.typeDescriptor = typeDescriptor;
    return symbol;
}

SymbolTableEntryPtr newVariable(int level, char* identifier, int displacement, TypeDescriptorPtr typeDescriptor) {
    VariableDescriptorPtr variableDescriptor = malloc(sizeof(VariableDescriptor));
    variableDescriptor->displacement = displacement;
    variableDescriptor->type = typeDescriptor;

    SymbolTableEntryPtr symbol = malloc(sizeof(SymbolTableEntry));
    symbol->category = VARIABLE_SYMBOL;
    symbol->level = level;
    symbol->identifier = identifier;
    symbol->description.variableDescriptor = variableDescriptor;

    return symbol;
}

TypeDescriptorPtr newArrayType(int size, int dimension, TypeDescriptorPtr elementType) {
    ArrayDescriptorPtr arrayDescriptor = malloc(sizeof(ArrayDescriptor));
    arrayDescriptor->dimension = dimension;
    arrayDescriptor->elementType = elementType;

    TypeDescriptorPtr typeDescriptor = malloc(sizeof(TypeDescriptorPtr));
    typeDescriptor->category = ARRAY_TYPE;
    typeDescriptor->size = size;

    return typeDescriptor;

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

void changeToLevelScope(SymbolTablePtr symbolTablePtr, int level) {
    Stack* auxStack = newStack();

    SymbolTableEntryPtr lastPoped = pop(symbolTablePtr->stack);
    while (lastPoped != NULL && lastPoped->level > level) {
        if (lastPoped->category == FUNCTION_SYMBOL) {
            if(lastPoped->level == level + 1) {
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

int parametersTotalSize(SymbolTableEntryPtr entry) {
    if (entry->category != FUNCTION_SYMBOL) {
        fprintf(stderr, "parametersTotalSize: Expected %s, got: %s\n",
                getSymbolTableCategoryName(FUNCTION_SYMBOL), getSymbolTableCategoryName(entry->category));
        exit(0);
    }

    int parametersTotalSize = 0;

    List* parameters = entry->description.functionDescriptor->params;

    LinkedNode* current = parameters->front;
    while (current != NULL) {
        SymbolTableEntryPtr entry = (SymbolTableEntryPtr) current->data;
        parametersTotalSize += entry->description.parameterDescriptor->type->size;

        current = current->next;
    }

    return parametersTotalSize;
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

/* Debug */
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