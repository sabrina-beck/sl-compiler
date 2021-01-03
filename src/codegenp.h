#include "codegen.h"
#include "tree.h"
#include "slc.h"
#include "symboltable.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

void processMainFunction(TreeNodePtr node);
void processFunction(TreeNodePtr node);

/* Function Header */
FunctionHeaderPtr processFunctionHeader(TreeNodePtr node);

TypeDescriptorPtr processFunctionReturnType(TreeNodePtr node);

ParameterPtr processFormalParameter(TreeNodePtr node);
ParameterPtr processParameterByReference(TreeNodePtr node);
ParameterPtr processParameterByValue(TreeNodePtr node);
ParameterPtr processParameter(TreeNodePtr node, ParameterPassage passage);
ParameterPtr processFunctionParameter(TreeNodePtr node);

/* Block */
void processBlock(TreeNodePtr node);

void processLabels(TreeNodePtr node);

void processTypes(TreeNodePtr node);
void processTypeDeclaration(TreeNodePtr node);

void processVariables(TreeNodePtr node);
void processVariableDeclaration(TreeNodePtr node);

void processFunctions(TreeNodePtr node);

/* Identifier & Type */
TypeDescriptorPtr processIdentifierAsType(TreeNodePtr node);
char* processIdentifier(TreeNodePtr node);

TypeDescriptorPtr processType(TreeNodePtr node);
TypeDescriptorPtr processArraySizeDeclaration(TreeNodePtr node, TypeDescriptorPtr elementType);

/* Body */
void processBody(TreeNodePtr node);

void processStatement(TreeNodePtr node);

void processLabel(TreeNodePtr node);

void processUnlabeledStatement(TreeNodePtr node);

void processAssignment(TreeNodePtr node);
Value processValue(TreeNodePtr node);
void loadArrayBaseAddress(Value value);
void processArrayIndexList(TreeNodePtr node, Value* variable);
void processArrayIndex(TreeNodePtr node, Value* value);

TypeDescriptorPtr processFunctionCall(TreeNodePtr node);

TypeDescriptorPtr processFunctionParameterCall(TreeNodePtr node, SymbolTableEntryPtr functionEntry);
TypeDescriptorPtr processRegularFunctionCall(TreeNodePtr node, SymbolTableEntryPtr functionEntry);
TypeDescriptorPtr processPseudoFunctionCall(TreeNodePtr node, SymbolTableEntryPtr functionEntry);
void processReadFunctionCall(TreeNodePtr argumentNode);
void processWriteFunctionCall(TreeNodePtr argumentNode);

void processArgumentsList(TreeNodePtr node, ParameterDescriptorsListPtr parameters);
void processArgumentByValue(ParameterDescriptorPtr expectedParameter, TreeNodePtr node);
void processArgumentByReference(ParameterDescriptorPtr expectedParameter, TreeNodePtr node);
void processArgumentByFunctionAsParameter(ParameterDescriptorPtr expectedParameter, TreeNodePtr node);
void processDeclaredFunctionAsArgument(ParameterDescriptorPtr expectedParameter, SymbolTableEntryPtr valueEntry);
void processFunctionParameterAsArgument(ParameterDescriptorPtr expectedParameter, SymbolTableEntryPtr valueEntry);

TreeNodePtr getVariableExpression(TreeNodePtr node);

void processGoto(TreeNodePtr node);

void processReturn(TreeNodePtr node);
void processReturnWithValue(TreeNodePtr expressionNode, FunctionDescriptorPtr functionDescriptor);

void processConditional(TreeNodePtr node);
void processRepetitive(TreeNodePtr node);

void processCompound(TreeNodePtr node);
void processUnlabeledStatementList(TreeNodePtr node);

TypeDescriptorPtr processExpression(TreeNodePtr node);
TypeDescriptorPtr routeExpressionSubtree(TreeNodePtr node);
TypeDescriptorPtr processBinaryOpExpression(TreeNodePtr node);
TypeDescriptorPtr processUnopExpression(TreeNodePtr node);
TypeDescriptorPtr processTerm(TreeNodePtr node);
TypeDescriptorPtr processFactor(TreeNodePtr node);
TypeDescriptorPtr processValueFactor(TreeNodePtr node);
TypeDescriptorPtr processIntegerFactor(TreeNodePtr node);
TypeDescriptorPtr processFunctionCallFactor(TreeNodePtr node);
int processInteger(TreeNodePtr node);
TypeDescriptorPtr processRelationalOperator(TreeNodePtr node);
TypeDescriptorPtr processAdditiveOperator(TreeNodePtr node);
TypeDescriptorPtr processUnaryOperator(TreeNodePtr node);
TypeDescriptorPtr processMultiplicativeOperator(TreeNodePtr node);

/** Error Handling **/
void LabelAlreadyDefinedError(char* identifier);
void UndeclaredLabelError(char* identifier);
void UnexpectedSymbolEntryCategoryError(SymbolTableCategory expected, SymbolTableCategory gotten);
void SymbolEntryCategoryError(char* expected, SymbolTableCategory gotten);
void UnexpectedNodeCategoryError(NodeCategory expected, NodeCategory gotten);
void UnexpectedChildNodeCategoryError(NodeCategory fatherNodeCategory, NodeCategory childNodeCategory);

/** Generates the final commands **/
void addCommand(const char* commandFormat, ...);