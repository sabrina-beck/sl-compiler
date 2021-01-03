#ifndef UTILS_HEADER
#define UTILS_HEADER

#include <stdlib.h>
#include "utils.h"

/***********************************************************************************************************************
 * Symbol Table definitions
 **********************************************************************************************************************/

typedef enum {
    VALUE_PARAMETER,
    VARIABLE_PARAMETER,
    FUNCTION_PARAMETER
} ParameterPassage;

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

typedef enum {
    TYPE_SYMBOL, // boolean and integer
    CONSTANT_SYMBOL, // true and false
    VARIABLE_SYMBOL,
    PARAMETER_SYMBOL,
    PSEUDO_FUNCTION_SYMBOL, // deal with read and write psudo functions
    FUNCTION_SYMBOL,
    LABEL_SYMBOL,
} SymbolTableCategory;

struct _TypeDescriptor;
struct _ParameterDescriptor;
struct _ParametersList;

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
    int headerMepaLabel;
    int bodyMepaLabel;
    int returnMepaLabel;
    int variablesDisplacement;
    int parametersSize;
    int returnDisplacement;
    TypeDescriptorPtr returnType;
    ParameterDescriptorsListPtr parameters;
    TypeDescriptorPtr functionType;
} FunctionDescriptor, *FunctionDescriptorPtr;

typedef struct {
    int mepaLabel;
    bool defined;
} LabelDescriptor, *LabelDescriptorPtr;

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

/***********************************************************************************************************************
 * Auxiliary structures
 **********************************************************************************************************************/

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

/***********************************************************************************************************************
 * Symbol Table functions
 **********************************************************************************************************************/
/* Returns the symbol table, this symbol table implementation is not thread safe! */
SymbolTablePtr getSymbolTable();

/* Returns the symbol table entry for the specific identifier */
SymbolTableEntryPtr findIdentifier(char* identifier);
/* Returns the function descriptor of the function being currently compiled */
FunctionDescriptorPtr findCurrentFunctionDescriptor();

/* Creates new function type descriptor */
TypeDescriptorPtr newFunctionType(FunctionHeaderPtr functionHeader);
/* Creates new array type descriptor */
TypeDescriptorPtr newArrayType(int dimension, TypeDescriptorPtr elementType);

/* Adds a new function entry to the symbol table */
SymbolTableEntryPtr addFunction(FunctionHeaderPtr functionHeader);
/* Adds a new function entry to the symbol table */
FunctionDescriptorPtr addMainFunction();
/* Adds a new label entry to the symbol table */
void addLabel(char* identifier);
/* Adds a new type entry to the symbol table */
void addType(char* identifier, TypeDescriptorPtr typeDescriptor);
/* Adds a new variable entry to the symbol table */
void addVariable(char* identifier, TypeDescriptorPtr typeDescriptor);

/***********************************************************************************************************************
 * Level counter functions
 **********************************************************************************************************************/
/* Gets the current function level counter value. Attention to the fact that the function level counter isn't thread safe */
int getFunctionLevel();
/*
 * It should be called when a function compilation is finished and the compiler will continue to compile the
 * outermost function.
 * It will remove all the symbols in the symbol table that are not accessible outside the compiled function and it will
 * update the function level counter.
 */
void endFunctionLevel();

/***********************************************************************************************************************
 * MEPA label counter functions
 **********************************************************************************************************************/
/* A MEPA label is simply an integer counter which returns a new integer every time, it isn't thread safe */
int nextMEPALabel();

/***********************************************************************************************************************
 * Type Compatibility functions
 **********************************************************************************************************************/
/* It checks if both types are compatible */
bool equivalentTypes(TypeDescriptorPtr type1, TypeDescriptorPtr type2);

/***********************************************************************************************************************
 * Auxiliary structures functions
 **********************************************************************************************************************/
/* Concatenates two parameters list, parameters1 will be at the beginning */
ParameterPtr concatenateParameters(ParameterPtr parameters1, ParameterPtr parameters2);
/* Creates a Value from a Symbol Table Entry */
Value valueFromEntry(SymbolTableEntryPtr entry);
/* Frees function header memory */
void freeFunctionHeader(FunctionHeaderPtr functionHeader);

/***********************************************************************************************************************
 * Debug facilities --> helpers to print errors
 **********************************************************************************************************************/
char* getSymbolTableCategoryName(SymbolTableCategory category);
char* getTypeCategoryName(TypeCategory category);
char* getPredefinedTypeName(PredefinedType category);

#endif