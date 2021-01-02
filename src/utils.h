#ifndef UTILS_HEADER
#define UTILS_HEADER

#include <stdlib.h>

#include "datastructures.h"

typedef enum {
    VALUE_PARAMETER,
    VARIABLE_PARAMETER,
    FUNCTION_PARAMETER
} ParameterPassage;

struct _TypeDescriptor;
struct _ParameterDescriptor;

typedef enum {
    PREDEFINED_TYPE,
    ARRAY_TYPE,
    FUNCTION_TYPE
} TypeCategory;

typedef enum {
    INTEGER,
    BOOLEAN
} PredefinedType;

typedef enum {
    READ,
    WRITE
} PseudoFunction;

typedef struct {
    int dimension; // number of elements
    struct _TypeDescriptor* elementType;
} ArrayDescriptor, *ArrayDescriptorPtr;

typedef struct {
    struct _TypeDescriptor* returnType;
    List* params; // List of SymbolTableEntry of category PARAMETER_SYMBOL
} FunctionTypeDescriptor, *FunctionTypeDescriptorPtr;

typedef struct _TypeDescriptor {
    TypeCategory category;
    int size;
    union { // depends on constr
        PredefinedType predefinedType;
        ArrayDescriptorPtr arrayDescriptor;
        FunctionTypeDescriptorPtr functionTypeDescriptor;
    } description;
} TypeDescriptor, *TypeDescriptorPtr;

typedef struct {
    int value;
    TypeDescriptorPtr type;
} ConstantDescriptor, *ConstantDescriptorPtr;

typedef struct {
    int displacement;
    TypeDescriptorPtr type;
} VariableDescriptor, *VariableDescriptorPtr;

typedef struct _ParameterDescriptor {
    int displacement;
    TypeDescriptorPtr type;
    ParameterPassage parameterPassage;
} ParameterDescriptor, *ParameterDescriptorPtr;

typedef struct {
    char* mepaLabel;
    char* returnLabel;
    int variablesDisplacement;
    int parametersSize;
    int returnDisplacement;
    TypeDescriptorPtr returnType;
    List* params; // List of SymbolTableEntry of category PARAMETER_SYMBOL
} FunctionDescriptor, *FunctionDescriptorPtr;

typedef struct {
    char* mepaLabel;
    bool defined;
} LabelDescriptor, *LabelDescriptorPtr;

typedef enum {
    TYPE_SYMBOL, // boolean and integer
    CONSTANT_SYMBOL, // true and false
    VARIABLE_SYMBOL,
    PARAMETER_SYMBOL,
    PSEUDO_FUNCTION_SYMBOL, // deal with read and write psudo functions
    FUNCTION_SYMBOL,
    LABEL_SYMBOL,
} SymbolTableCategory;

typedef struct {
    SymbolTableCategory category;
    char* identifier;
    int level;
    union { // it depends on category type
        TypeDescriptorPtr typeDescriptor;
        ConstantDescriptorPtr constantDescriptor;
        VariableDescriptorPtr variableDescriptor;
        ParameterDescriptorPtr parameterDescriptor;
        FunctionDescriptorPtr functionDescriptor;
        LabelDescriptorPtr labelDescriptor;
        PseudoFunction pseudoFunction;
    } description;
} SymbolTableEntry, *SymbolTableEntryPtr;

typedef struct {
    Stack* stack;
    FunctionDescriptorPtr mainFunctionDescriptor;
    TypeDescriptorPtr integerTypeDescriptor;
    TypeDescriptorPtr booleanTypeDescriptor;
} SymbolTable, *SymbolTablePtr;

/****
 *
 ****/

struct _FunctionHeader;

typedef struct _Parameter {
    char* name;
    ParameterPassage passage;
    struct _Parameter* next;
    TypeDescriptorPtr type;
} Parameter, *ParameterPtr;

typedef struct _FunctionHeader {
    TypeDescriptorPtr returnType;
    char* name;
    ParameterPtr parameters;
} FunctionHeader, *FunctionHeaderPtr;

ParameterPtr concatenateParameters(ParameterPtr parameters1, ParameterPtr parameters2);

typedef enum {
    CONSTANT,
    ADDRESS,
    VALUE,
    ARRAY
} VariableCategory;

typedef struct {
    VariableCategory category;
    TypeDescriptorPtr type;
    int level;
    int displacement;
    int value;
} Variable, *VariablePtr;

/****
 *
 ****/

SymbolTablePtr initializeSymbolTable();

SymbolTableEntryPtr findIdentifier(SymbolTablePtr symbolTable, char* identifier);
FunctionDescriptorPtr findCurrentFunctionDescriptor(SymbolTablePtr symbolTable);

TypeDescriptorPtr newFunctionType(FunctionHeaderPtr functionHeader);
TypeDescriptorPtr newArrayType(int dimension, TypeDescriptorPtr elementType);

void addMainFunction(SymbolTablePtr symbolTable);
SymbolTableEntryPtr addFunction(SymbolTablePtr symbolTable, FunctionHeaderPtr functionHeader);
void addLabel(SymbolTablePtr symbolTable, char* identifier);
void addType(SymbolTablePtr symbolTable, char* identifier, TypeDescriptorPtr typeDescriptor);
void addVariable(SymbolTablePtr symbolTable, char* identifier, TypeDescriptorPtr typeDescriptor);

void addSymbolTableEntry(SymbolTablePtr symbolTable, SymbolTableEntryPtr entry);

int getFunctionLevel();
void endFunctionLevel(SymbolTablePtr symbolTablePtr);


int parametersTotalSize(SymbolTableEntryPtr entry); // entry of category FUNCTION_SYMBOL

bool equivalentTypes(TypeDescriptorPtr type1, TypeDescriptorPtr type2);
bool equivalentFunctions(TypeDescriptorPtr functionType, FunctionDescriptorPtr functionDescriptor);

char* nextMEPALabel();


/* Debug */
char* getSymbolTableCategoryName(SymbolTableCategory category);

#endif