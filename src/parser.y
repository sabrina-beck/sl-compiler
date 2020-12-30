%{

#include "tree.h"
#include<stdio.h>

/* FLEX functions */
extern int yylex (void);
void yyerror(char *);
char *tokenValue;

%}

%{ /* Comparison operators */ %}
%token EQUAL
%token DIFFERENT
%token LESS
%token LESS_OR_EQUAL
%token GREATER
%token GREATER_OR_EQUAL

%{ /* Integer Operations */ %}
%token PLUS
%token MINUS
%token MULTIPLY
%token DIV

%{ /* Boolean Operations */ %}
%token OR
%token AND
%token NOT

%{ /* Ponctuation */ %}
%token OPEN_BRACE
%token CLOSE_BRACE
%token OPEN_BRACKET
%token CLOSE_BRACKET
%token OPEN_PAREN
%token CLOSE_PAREN
%token COLON
%token COMMA
%token SEMI_COLON

%{ /* Assignment */ %}
%token ASSIGN

%{ /* Reserved words */ %}
%token ELSE
%token FUNCTIONS
%token GOTO
%token IF
%token LABELS
%token RETURN
%token TYPES
%token VAR
%token VARS
%token VOID
%token WHILE

%{ /* End of file */ %}
%token END_OF_FILE

%{ /* Integers */ %}
%token INTEGER

%{ /* Identifiers */ %}
%token IDENTIFIER

%{ /* Lexical error */ %}
%token LEXICAL_ERROR

%%
program                     : function END_OF_FILE {return 0;}
                            ;

function                    : function_header block { addTreeNode(FUNCTION_NODE, 2); }
                            ;


function_header             : function_return_type identifier formal_parameters { addTreeNode(FUNCTION_HEADER_NODE, 3); }
                            ;
function_return_type        : VOID { addEmpty(); }
                            | identifier
                            ;
formal_parameters           : OPEN_PAREN CLOSE_PAREN { addEmpty(); }
                            | OPEN_PAREN formal_parameter_list CLOSE_PAREN
                            ;
formal_parameter_list       : formal_parameter
                            | formal_parameter COMMA formal_parameter_list { addSequence(); }
                            ;
formal_parameter            : expression_parameter
                            | function_parameter
                            ;
expression_parameter        : VAR identifier_list COLON identifier { addTreeNode(EXPRESSION_PARAMETER_BY_REFERENCE_NODE, 2); }
                            | identifier_list COLON identifier { addTreeNode(EXPRESSION_PARAMETER_BY_VALUE_NODE, 2); }
                            ;
function_parameter          : function_header { addTreeNode(FUNCTION_PARAMETER_NODE, 1); }
                            ;


block                       : labels_section types_section variables_section functions_section body { addTreeNode(BLOCK_NODE, 5); }
                            ;

labels_section              : { addEmpty(); }
                            | labels
                            ;
labels                      : LABELS identifier_list SEMI_COLON { addTreeNode(LABELS_NODE, 1); }
                            ;

types_section               : { addEmpty(); }
                            | types
                            ;
types                       : TYPES type_declaration_list { addTreeNode(TYPES_NODE, 1); }
                            ;
type_declaration_list       : type_declaration
                            | type_declaration type_declaration_list { addSequence(); }
                            ;
type_declaration            : identifier ASSIGN type SEMI_COLON { addTreeNode(TYPE_DECLARATION_NODE, 2); }
                            ;

variables_section           : { addEmpty(); }
                            | variables
                            ;
variables                   : VARS declarations_list { addTreeNode(VARIABLES_NODE, 1); }
                            ;
declarations_list           : declaration SEMI_COLON
                            | declaration SEMI_COLON declarations_list { addSequence(); }
                            ;
declaration                 : identifier_list COLON type { addTreeNode(DECLARATION_NODE, 2); }
                            ;

functions_section           : { addEmpty(); }
                            | functions
                            ;
functions                   : FUNCTIONS functions_list { addTreeNode(FUNCTIONS_NODE, 1); }
                            ;
functions_list              : function
                            | function functions_list { addSequence(); }
                            ;


identifier_list             : identifier
                            | identifier COMMA identifier_list { addSequence(); }
                            ;
identifier                  : IDENTIFIER { addIdentifier(tokenValue); }
                            ;


type                        : identifier array_size_declaration_list { addTreeNode(TYPE_NODE, 2); }
                            ;
array_size_declaration_list : { addEmpty(); }
                            | array_size_declaration array_size_declaration_list { addSequence(); }
                            ;
array_size_declaration      : OPEN_BRACKET integer CLOSE_BRACKET { addTreeNode(ARRAY_SIZE_NODE, 1); }
                            ;


body                        : OPEN_BRACE statement_list CLOSE_BRACE { addTreeNode(BODY_NODE, 1); }
                            ;
statement_list              : { addEmpty(); }
                            | statement statement_list { addSequence(); }
                            ;
statement                   : label unlabeled_statement { addTreeNode(STATEMENT_NODE, 2); }
                            | unlabeled_statement { addTreeNode(STATEMENT_NODE, 1); }
                            ;
label                       : identifier COLON { addTreeNode(LABEL_NODE, 1); }
                            ;
unlabeled_statement         : assignment
                            | function_call_statement
                            | goto
                            | return
                            | conditional
                            | repetitive
                            | compound
                            | empty_statement
                            ;

assignment                  : variable ASSIGN expression SEMI_COLON { addTreeNode(ASSIGNMENT_NODE, 2); }
                            ;
variable                    : identifier array_index_list { addTreeNode(VARIABLE_NODE, 2); }
                            ;
array_index_list            : { addEmpty(); }
                            | array_index array_index_list { addSequence(); }
                            ;
array_index                 : OPEN_BRACKET expression CLOSE_BRACKET { addTreeNode(ARRAY_INDEX_NODE, 1); }
                            ;

function_call_statement     : function_call SEMI_COLON
                            ;
function_call               : identifier OPEN_PAREN expression_list CLOSE_PAREN { addTreeNode(FUNCTION_CALL_NODE, 2); }
expression_list             : { addEmpty(); }
                            | expression
                            | expression COMMA expression_list { addSequence(); }
                            ;

goto                        : GOTO identifier SEMI_COLON { addTreeNode(GOTO_NODE, 1); }
                            ;

return                      : RETURN SEMI_COLON { addTreeNode(RETURN_NODE, 0); }
                            | RETURN expression SEMI_COLON { addTreeNode(RETURN_NODE, 1); }
                            ;

conditional                 : IF OPEN_PAREN expression CLOSE_PAREN compound { addTreeNode(IF_NODE, 2); }
                            | IF OPEN_PAREN expression CLOSE_PAREN compound ELSE compound { addTreeNode(IF_NODE, 3); }
                            ;

repetitive                  : WHILE OPEN_PAREN expression CLOSE_PAREN compound { addTreeNode(WHILE_NODE, 2); }
                            ;

compound                    : OPEN_BRACE unlabeled_statement_list CLOSE_BRACE { addTreeNode(COMPOUND_NODE, 1); }
                            ;
unlabeled_statement_list    : { addEmpty(); }
                            | unlabeled_statement unlabeled_statement_list { addSequence(); }
                            ;

empty_statement             : SEMI_COLON { addEmpty(); }

expression                  : binaryop_expression { addTreeNode(EXPRESSION_NODE, 1); }
                            | binaryop_expression relational_operator binaryop_expression { addTreeNode(EXPRESSION_NODE, 3); }
                            | unop_expression { addTreeNode(EXPRESSION_NODE, 1); }
                            | unop_expression relational_operator binaryop_expression { addTreeNode(EXPRESSION_NODE, 3); }
                            ;
binaryop_expression         : term additive_operation { addTreeNode(BINARY_OPERATOR_EXPRESSION_NODE, 2); }
                            ;
unop_expression             : unary_operator term additive_operation { addTreeNode(UNARY_OPERATOR_EXPRESSION_NODE, 3); }
                            ;
additive_operation          : { addEmpty(); }
                            | additive_operator term additive_operation { addTreeNode(ADDITIVE_OPERATION_NODE, 3); }
                            ;

term                        : factor multiplicative_operation { addTreeNode(TERM_NODE, 2); }
                            ;
multiplicative_operation    : { addEmpty(); }
                            | multiplicative_operator factor multiplicative_operation { addTreeNode(MULTIPLICATIVE_OPERATION_NODE, 3); }
                            ;

factor                      : variable { addTreeNode(FACTOR_NODE, 1); }
                            | integer { addTreeNode(FACTOR_NODE, 1); }
                            | function_call { addTreeNode(FACTOR_NODE, 1); }
                            | OPEN_PAREN expression CLOSE_PAREN { addTreeNode(FACTOR_NODE, 1); }
                            ;


integer                     : INTEGER { addTreeNodeWithName(INTEGER_NODE, 0, tokenValue); }
                            ;


relational_operator         : LESS_OR_EQUAL { addTreeNode(LESS_OR_EQUAL_NODE, 0), addTreeNode(RELATIONAL_OPERATOR_NODE, 1); }
                            | LESS { addTreeNode(LESS_NODE, 0), addTreeNode(RELATIONAL_OPERATOR_NODE, 1); }
                            | EQUAL { addTreeNode(EQUAL_NODE, 0), addTreeNode(RELATIONAL_OPERATOR_NODE, 1); }
                            | DIFFERENT { addTreeNode(DIFFERENT_NODE, 0), addTreeNode(RELATIONAL_OPERATOR_NODE, 1); }
                            | GREATER_OR_EQUAL { addTreeNode(GREATER_OR_EQUAL_NODE, 0), addTreeNode(RELATIONAL_OPERATOR_NODE, 1); }
                            | GREATER { addTreeNode(GREATER_NODE, 0), addTreeNode(RELATIONAL_OPERATOR_NODE, 1); }
                            ;
additive_operator           : PLUS { addTreeNode(PLUS_NODE, 0), addTreeNode(ADDITIVE_OPERATOR_NODE, 1); }
                            | MINUS { addTreeNode(MINUS_NODE, 0), addTreeNode(ADDITIVE_OPERATOR_NODE, 1); }
                            | OR { addTreeNode(OR_NODE, 0), addTreeNode(ADDITIVE_OPERATOR_NODE, 1); }
                            ;
unary_operator              : PLUS { addTreeNode(PLUS_NODE, 0), addTreeNode(UNARY_OPERATOR_NODE, 1); }
                            | MINUS { addTreeNode(MINUS_NODE, 0), addTreeNode(UNARY_OPERATOR_NODE, 1); }
                            | NOT { addTreeNode(NOT_NODE, 0), addTreeNode(UNARY_OPERATOR_NODE, 1); }
                            ;
multiplicative_operator     : MULTIPLY { addTreeNode(MULTIPLY_NODE, 0), addTreeNode(MULTIPLICATIVE_OPERATOR_NODE, 1); }
                            | DIV { addTreeNode(DIV_NODE, 0), addTreeNode(MULTIPLICATIVE_OPERATOR_NODE, 1); }
                            | AND { addTreeNode(AND_NODE, 0), addTreeNode(MULTIPLICATIVE_OPERATOR_NODE, 1); }
                            ;
%%
