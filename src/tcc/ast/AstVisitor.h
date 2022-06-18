#ifndef TINYC_ASTVISITOR_H
#define TINYC_ASTVISITOR_H

#include "pipeline/PipelineStage.h"
#include "TinyCBaseVisitor.h"

class AstVisitor : public TinyCBaseVisitor,
                   public PipeModifierBase {
public:
    std::any modify(std::any data) override;

    antlrcpp::Any visitTranslationUnit(TinyCParser::TranslationUnitContext* ctx) override;
    antlrcpp::Any visitStructDef(TinyCParser::StructDefContext* ctx) override;
    antlrcpp::Any visitFunctionDef(TinyCParser::FunctionDefContext* ctx) override;
    antlrcpp::Any visitStatements(TinyCParser::StatementsContext* ctx) override;
    antlrcpp::Any visitStatement(TinyCParser::StatementContext* ctx) override;
    antlrcpp::Any visitIfStatement(TinyCParser::IfStatementContext* ctx) override;
    antlrcpp::Any visitVariableDecl(TinyCParser::VariableDeclContext* ctx) override;
    antlrcpp::Any visitAssignment(TinyCParser::AssignmentContext* ctx) override;
    antlrcpp::Any visitReturnStatement(TinyCParser::ReturnStatementContext* ctx) override;
    antlrcpp::Any visitWhileStatement(TinyCParser::WhileStatementContext* ctx) override;
    antlrcpp::Any visitForStatement(TinyCParser::ForStatementContext* ctx) override;

    antlrcpp::Any visitCompExpression(TinyCParser::CompExpressionContext* ctx) override;
    antlrcpp::Any visitAddSubExpr(TinyCParser::AddSubExprContext* ctx) override;
    antlrcpp::Any visitMulDivExpr(TinyCParser::MulDivExprContext* ctx) override;
    antlrcpp::Any visitIndexedOperand(TinyCParser::IndexedOperandContext* ctx) override;
    antlrcpp::Any visitOperandDereference(TinyCParser::OperandDereferenceContext* ctx) override;
    antlrcpp::Any visitFieldAccess(TinyCParser::FieldAccessContext* ctx) override;
    antlrcpp::Any visitCallExpr(TinyCParser::CallExprContext* ctx) override;
    antlrcpp::Any visitLiteral(TinyCParser::LiteralContext* ctx) override;
    antlrcpp::Any visitValueReference(TinyCParser::ValueReferenceContext* ctx) override;
    antlrcpp::Any visitVariableName(TinyCParser::VariableNameContext* ctx) override;
};

#endif
