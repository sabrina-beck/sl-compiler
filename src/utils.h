#ifndef UTILS_HEADER
#define UTILS_HEADER

#include <stdlib.h>

#include "datastructures.h"

/****
 * Symbol Table definitions
 ****/

typedef enum {
    VALUE_PARAMETER,
    VARIABLE_PARAMETER,
    FUNCTION_PARAMETER
} ParameterPassage;

struct _TypeDescriptor;
struct _ParameterDescriptor;
struct _ParametersList;

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
    struct _ParametersList* parameters;
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

typedef struct _ParametersList {
    ParameterDescriptorPtr descriptor;
    struct _ParametersList* next;
} ParameterDescriptorsList, *ParameterDescriptorsListPtr;

typedef struct {
    char* mepaLabel;
    char* returnLabel;
    int variablesDisplacement;
    int parametersSize;
    int returnDisplacement;
    TypeDescriptorPtr returnType;
    ParameterDescriptorsListPtr parameters;
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
 * Other definitions
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

typedef enum {
    CONSTANT,
    REFERENCE,
    VALUE,
    ARRAY_VALUE,
    ARRAY_REFERENCE
} ValueCategory;

typedef struct {
    ValueCategory category;
    TypeDescriptorPtr type;
    int level;
    union {
        int displacement; // REFERENCE, VALUE, ARRAY_VALUE, ARRAY_REFERENCE
        int value; // CONSTANTS
    } content;
} Value;

/****
 * Symbol Table functions
 ****/

SymbolTablePtr initializeSymbolTable();

SymbolTableEntryPtr findIdentifier(SymbolTablePtr symbolTable, char* identifier);
FunctionDescriptorPtr findCurrentFunctionDescriptor(SymbolTablePtr symbolTable);

TypeDescriptorPtr newFunctionType(FunctionHeaderPtr functionHeader);
TypeDescriptorPtr newArrayType(int dimension, TypeDescriptorPtr elementType);

SymbolTableEntryPtr addFunction(SymbolTablePtr symbolTable, FunctionHeaderPtr functionHeader);
void addMainFunction(SymbolTablePtr symbolTable);
void addLabel(SymbolTablePtr symbolTable, char* identifier);
void addType(SymbolTablePtr symbolTable, char* identifier, TypeDescriptorPtr typeDescriptor);
void addVariable(SymbolTablePtr symbolTable, char* identifier, TypeDescriptorPtr typeDescriptor);

void endFunctionLevel(SymbolTablePtr symbolTablePtr);

bool equivalentTypes(TypeDescriptorPtr type1, TypeDescriptorPtr type2);
bool equivalentFunctions(TypeDescriptorPtr functionType, FunctionDescriptorPtr functionDescriptor);

//...
void addSymbolTableEntry(SymbolTablePtr symbolTable, SymbolTableEntryPtr entry);

int getFunctionLevel();

char* nextMEPALabel();

/****
 * Other functions
 ****/

ParameterPtr concatenateParameters(ParameterPtr parameters1, ParameterPtr parameters2);
Value valueFromEntry(SymbolTableEntryPtr entry);

/****
 * Debug
 ****/
char* getSymbolTableCategoryName(SymbolTableCategory category);

#endif