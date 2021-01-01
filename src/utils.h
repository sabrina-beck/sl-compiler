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
    TypeDescriptorPtr integerTypeDescriptor;
    TypeDescriptorPtr booleanTypeDescriptor;
} SymbolTable, *SymbolTablePtr;

SymbolTablePtr initializeSymbolTable();

SymbolTableEntryPtr findIdentifier(SymbolTablePtr symbolTable, char* identifier);

SymbolTableEntryPtr newParameter(int level, char* identifier, int displacement, TypeDescriptorPtr type, ParameterPassage parameterPassage);
SymbolTableEntryPtr newFunctionParameter(SymbolTableEntryPtr functionEntry, int displacement);
SymbolTableEntryPtr newFunctionDescriptor(int level, char* identifier, TypeDescriptorPtr returnType, List* paramEntries); // list of parameters as SymbolTableEntryPtr
SymbolTableEntryPtr newLabel(int level, char* identifier);
SymbolTableEntryPtr newType(int level, char* identifier, TypeDescriptorPtr typeDescriptor);
SymbolTableEntryPtr newVariable(int level, char* identifier, int displacement, TypeDescriptorPtr typeDescriptor);
void addSymbolTableEntry(SymbolTablePtr symbolTable, SymbolTableEntryPtr entry);

int parametersTotalSize(SymbolTableEntryPtr entry); // entry of category FUNCTION_SYMBOL

bool equivalentTypes(TypeDescriptorPtr type1, TypeDescriptorPtr type2);

char* nextMEPALabel();

