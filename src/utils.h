#include <stdlib.h>

typedef enum {false, true} bool;

typedef enum {
    VALUE_PARAMETER,
    VARIABLE_PARAMETER
} ParameterPassage;

typedef enum {
    TYPE_SYMBOL, // boolean and integer
    CONSTANT_SYMBOL, // true and false
    VARIABLE_SYMBOL,
    PARAMETER_SYMBOL,
    FUNCTION_SYMBOL, // where are read and write?????
    LABEL_SYMBOL,
} SymbolTableCategory;

struct _TypeDescriptor;
struct _ParameterDescriptor;

typedef enum {
    PREDEFINED_TYPE,
    ARRAY_TYPE,
    FUNCTION_TYPE
} TypeConstr;

typedef enum {
    INTEGER,
    BOOLEAN
} PredefinedType;

typedef struct {
    int dimension; // number of elements
    struct _TypeDescriptor* elementType;
} ArrayDescriptor, *ArrayDescriptorPtr;

typedef struct {
    struct _TypeDescriptor* result;
    struct _ParameterDescriptor* params;
} FunctionTypeDescriptor, *FunctionTypeDescriptorPtr;

typedef struct _TypeDescriptor {
    TypeConstr constr;
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
    int displacement; ///????
    TypeDescriptorPtr type;
    ParameterDescriptorPtr params;
} FunctionDescriptor, *FunctionDescriptorPtr;

typedef struct {
    char* mepaLabel;
    bool defined;
} LabelDescriptor, *LabelDescriptorPtr;

typedef struct _symbolTableEntry {
    SymbolTableCategory category;
    char* identifier;
    int level;
    struct _symbolTableEntry* next;
    union { // it depends on category type
        ConstantDescriptorPtr constantDescriptor;
        VariableDescriptorPtr variableDescriptor;
        ParameterDescriptorPtr parameterDescriptor;
        FunctionDescriptorPtr functionDescriptor;
        LabelDescriptorPtr labelDescriptor;
        TypeDescriptorPtr typeDescriptor;
    } description;
} SymbolTableEntry, *SymbolTableEntryPtr;

// TODO SymbolTableEntryPtr symbolTable = NULL; // initialization

char* nextMEPALabel();