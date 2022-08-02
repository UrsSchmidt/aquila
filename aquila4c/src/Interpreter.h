#pragma once

#include "../gen/AquilaVisitor.h"

class Interpreter : public AquilaVisitor {
public:

    std::any visitProgram(AquilaParser::ProgramContext *ctx);

    std::any visitStatement(AquilaParser::StatementContext *ctx);

    std::any visitIfStatement(AquilaParser::IfStatementContext *ctx);

    std::any visitSwitchStatement(AquilaParser::SwitchStatementContext *ctx);

    std::any visitSwitchStatementLabels(AquilaParser::SwitchStatementLabelsContext *ctx);

    std::any visitLoopStatement(AquilaParser::LoopStatementContext *ctx);

    std::any visitForStatement(AquilaParser::ForStatementContext *ctx);

    std::any visitReadStatement(AquilaParser::ReadStatementContext *ctx);

    std::any visitWriteStatement(AquilaParser::WriteStatementContext *ctx);

    std::any visitAssignStatement(AquilaParser::AssignStatementContext *ctx);

    std::any visitCallStatement(AquilaParser::CallStatementContext *ctx);

    std::any visitRemoveStatement(AquilaParser::RemoveStatementContext *ctx);

    std::any visitRunStatement(AquilaParser::RunStatementContext *ctx);

    std::any visitBlock(AquilaParser::BlockContext *ctx);

    std::any visitLhs(AquilaParser::LhsContext *ctx);

    std::any visitLhsPart(AquilaParser::LhsPartContext *ctx);

    std::any visitExpression(AquilaParser::ExpressionContext *ctx);

    std::any visitIfExpression(AquilaParser::IfExpressionContext *ctx);

    std::any visitLetExpression(AquilaParser::LetExpressionContext *ctx);

    std::any visitLetBindExpression(AquilaParser::LetBindExpressionContext *ctx);

    std::any visitSwitchExpression(AquilaParser::SwitchExpressionContext *ctx);

    std::any visitSwitchExpressionLabels(AquilaParser::SwitchExpressionLabelsContext *ctx);

    std::any visitLogicalOperation(AquilaParser::LogicalOperationContext *ctx);

    std::any visitLogicalOperator(AquilaParser::LogicalOperatorContext *ctx);

    std::any visitUnaryLogicalOperation(AquilaParser::UnaryLogicalOperationContext *ctx);

    std::any visitUnaryLogicalOperator(AquilaParser::UnaryLogicalOperatorContext *ctx);

    std::any visitRelation(AquilaParser::RelationContext *ctx);

    std::any visitRelationalOperator(AquilaParser::RelationalOperatorContext *ctx);

    std::any visitAddition(AquilaParser::AdditionContext *ctx);

    std::any visitAdditionalOperator(AquilaParser::AdditionalOperatorContext *ctx);

    std::any visitMultiplication(AquilaParser::MultiplicationContext *ctx);

    std::any visitMultiplicationOperator(AquilaParser::MultiplicationOperatorContext *ctx);

    std::any visitUnaryAddition(AquilaParser::UnaryAdditionContext *ctx);

    std::any visitUnaryAdditionalOperator(AquilaParser::UnaryAdditionalOperatorContext *ctx);

    std::any visitFactorial(AquilaParser::FactorialContext *ctx);

    std::any visitFactorialOperator(AquilaParser::FactorialOperatorContext *ctx);

    std::any visitFactor(AquilaParser::FactorContext *ctx);

    std::any visitBracketExpression(AquilaParser::BracketExpressionContext *ctx);

    std::any visitAbsExpression(AquilaParser::AbsExpressionContext *ctx);

    std::any visitDictAggregate(AquilaParser::DictAggregateContext *ctx);

    std::any visitDictAggregatePair(AquilaParser::DictAggregatePairContext *ctx);

    std::any visitDictAggregateKey(AquilaParser::DictAggregateKeyContext *ctx);

    std::any visitLambdaExpression(AquilaParser::LambdaExpressionContext *ctx);

    std::any visitLambdaExpressionParameter(AquilaParser::LambdaExpressionParameterContext *ctx);

    std::any visitLiteral(AquilaParser::LiteralContext *ctx);

    std::any visitFunctionCall(AquilaParser::FunctionCallContext *ctx);

    std::any visitAccessExpression(AquilaParser::AccessExpressionContext *ctx);

    std::any visitAccessExpressionPart(AquilaParser::AccessExpressionPartContext *ctx);

};
