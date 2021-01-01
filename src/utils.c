#include "utils.h"

#include <stdio.h>
#include <string.h>

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
    return strcmp(identifier, entry->identifier);
}

SymbolTableEntryPtr findIdentifier(SymbolTablePtr symbolTable, char* identifier) {
    return (SymbolTableEntryPtr) find(symbolTable->stack, identifier, byIdentifierPredicate);
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
        fprintf(stderr, "Expected function entry!");
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
    functionDescriptor->returnDisplacement = -paramEntries->size - 5; // FIXME explain
    functionDescriptor->returnType = returnType;
    functionDescriptor->params = paramEntries;

    SymbolTableEntryPtr symbol = malloc(sizeof(SymbolTableEntry));
    symbol->category = CONSTANT_SYMBOL;
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

bool equivalentTypes(TypeDescriptorPtr type1, TypeDescriptorPtr type2) {
    if(type1 == NULL || type2 == NULL) {
        return true;
    }

    return type1->category == PREDEFINED_TYPE && type2->category == PREDEFINED_TYPE &&
    type1->size == type2->size && type1->description.predefinedType == type2->description.predefinedType;
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