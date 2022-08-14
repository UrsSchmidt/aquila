#pragma once

#include "../gen/AquilaVisitor.h"

#include <any>
#include <filesystem>
#include <gmp.h>
#include <map>
#include <string>
#include <vector>

class DictComparator {
public:
    bool operator() (const std::any& a1, const std::any& a2) const;
};

typedef
    std::any
    Any;
typedef
    bool
    Boolean;
typedef
    struct Integer { mpz_t i; }
    Integer;
typedef
    std::string
    String;
typedef
    AquilaParser::LambdaExpressionContext*
    Function;
typedef
    std::map<std::string, Any, DictComparator>
    Dictionary;

class Interpreter : public AquilaVisitor {
private:

    char* script_caller;
    std::filesystem::path script_root;

    void handleLhs(AquilaParser::LhsContext* lhsc, std::function<void(Dictionary*, String)> handler);

    Any callFunction(AquilaParser::FunctionCallContext* callsite, Function& function, const std::vector<Any>& arguments);
    bool checkArgs(AquilaParser::FunctionCallContext* callsite, const std::vector<Any>& arguments, std::vector<std::string> types);
    bool checkArgsNoFail(AquilaParser::FunctionCallContext* callsite, const std::vector<Any>& arguments, std::vector<std::string> types);
    bool checkArgsHelper(AquilaParser::FunctionCallContext* callsite, const std::vector<Any>& arguments, bool failOnTypeMismatch, std::vector<std::string> types);

public:

    Interpreter(int argc, char* argv[]);

    Any visitProgram(AquilaParser::ProgramContext *ctx);

    Any visitStatement(AquilaParser::StatementContext *ctx);

    Any visitIfStatement(AquilaParser::IfStatementContext *ctx);

    Any visitSwitchStatement(AquilaParser::SwitchStatementContext *ctx);

    Any visitSwitchStatementLabels(AquilaParser::SwitchStatementLabelsContext *ctx);

    Any visitLoopStatement(AquilaParser::LoopStatementContext *ctx);

    Any visitForStatement(AquilaParser::ForStatementContext *ctx);

    Any visitCallStatement(AquilaParser::CallStatementContext *ctx);

    Any visitReadStatement(AquilaParser::ReadStatementContext *ctx);

    Any visitWriteStatement(AquilaParser::WriteStatementContext *ctx);

    Any visitRemoveStatement(AquilaParser::RemoveStatementContext *ctx);

    Any visitRunStatement(AquilaParser::RunStatementContext *ctx);

    Any visitAssignStatement(AquilaParser::AssignStatementContext *ctx);

    Any visitBlock(AquilaParser::BlockContext *ctx);

    Any visitLhs(AquilaParser::LhsContext *ctx);

    Any visitLhsPart(AquilaParser::LhsPartContext *ctx);

    Any visitExpression(AquilaParser::ExpressionContext *ctx);

    Any visitIfExpression(AquilaParser::IfExpressionContext *ctx);

    Any visitLetExpression(AquilaParser::LetExpressionContext *ctx);

    Any visitLetBindExpression(AquilaParser::LetBindExpressionContext *ctx);

    Any visitSwitchExpression(AquilaParser::SwitchExpressionContext *ctx);

    Any visitSwitchExpressionLabels(AquilaParser::SwitchExpressionLabelsContext *ctx);

    Any visitLogicalOperation(AquilaParser::LogicalOperationContext *ctx);

    Any visitLogicalOperator(AquilaParser::LogicalOperatorContext *ctx);

    Any visitUnaryLogicalOperation(AquilaParser::UnaryLogicalOperationContext *ctx);

    Any visitUnaryLogicalOperator(AquilaParser::UnaryLogicalOperatorContext *ctx);

    Any visitRelation(AquilaParser::RelationContext *ctx);

    Any visitRelationalOperator(AquilaParser::RelationalOperatorContext *ctx);

    Any visitAddition(AquilaParser::AdditionContext *ctx);

    Any visitAdditionalOperator(AquilaParser::AdditionalOperatorContext *ctx);

    Any visitMultiplication(AquilaParser::MultiplicationContext *ctx);

    Any visitMultiplicationOperator(AquilaParser::MultiplicationOperatorContext *ctx);

    Any visitUnaryAddition(AquilaParser::UnaryAdditionContext *ctx);

    Any visitUnaryAdditionalOperator(AquilaParser::UnaryAdditionalOperatorContext *ctx);

    Any visitFactorial(AquilaParser::FactorialContext *ctx);

    Any visitFactorialOperator(AquilaParser::FactorialOperatorContext *ctx);

    Any visitFactor(AquilaParser::FactorContext *ctx);

    Any visitBracketExpression(AquilaParser::BracketExpressionContext *ctx);

    Any visitAbsExpression(AquilaParser::AbsExpressionContext *ctx);

    Any visitDictAggregate(AquilaParser::DictAggregateContext *ctx);

    Any visitDictAggregatePair(AquilaParser::DictAggregatePairContext *ctx);

    Any visitDictAggregateKey(AquilaParser::DictAggregateKeyContext *ctx);

    Any visitLambdaExpression(AquilaParser::LambdaExpressionContext *ctx);

    Any visitLambdaExpressionParameter(AquilaParser::LambdaExpressionParameterContext *ctx);

    Any visitLiteral(AquilaParser::LiteralContext *ctx);

    Any visitFunctionCall(AquilaParser::FunctionCallContext *ctx);

    Any visitAccessExpression(AquilaParser::AccessExpressionContext *ctx);

    Any visitAccessExpressionPart(AquilaParser::AccessExpressionPartContext *ctx);

};
