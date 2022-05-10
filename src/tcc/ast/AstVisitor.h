#ifndef TINYC_ASTVISITOR_H
#define TINYC_ASTVISITOR_H

#include "TinyCBaseVisitor.h"

class AstVisitor : public TinyCBaseVisitor {
public:
    std::any visitTranslationUnit(TinyCParser::TranslationUnitContext* ctx) override;
    std::any visitFunction(TinyCParser::FunctionContext* ctx) override;
    std::any visitStatements(TinyCParser::StatementsContext* ctx) override;
    std::any visitStatement(TinyCParser::StatementContext* ctx) override;
    std::any visitIfStatement(TinyCParser::IfStatementContext* ctx) override;
    std::any visitVariableDecl(TinyCParser::VariableDeclContext* ctx) override;
    std::any visitAssignment(TinyCParser::AssignmentContext* ctx) override;
    std::any visitAssignable(TinyCParser::AssignableContext* ctx) override;
    std::any visitReturnStatement(TinyCParser::ReturnStatementContext* ctx) override;
    std::any visitWhileStatement(TinyCParser::WhileStatementContext* ctx) override;
    std::any visitForStatement(TinyCParser::ForStatementContext* ctx) override;

    std::any visitCompExpression(TinyCParser::CompExpressionContext* ctx) override;
    std::any visitAddSubExpr(TinyCParser::AddSubExprContext* ctx) override;
    std::any visitMulDivExpr(TinyCParser::MulDivExprContext* ctx) override;
    std::any visitIndexedOperand(TinyCParser::IndexedOperandContext* ctx) override;
    std::any visitOperandDereference(TinyCParser::OperandDereferenceContext* ctx) override;
    std::any visitCallExpr(TinyCParser::CallExprContext* ctx) override;
    std::any visitLiteral(TinyCParser::LiteralContext* ctx) override;
    std::any visitValueReference(TinyCParser::ValueReferenceContext* ctx) override;
    std::any visitVariableName(TinyCParser::VariableNameContext* ctx) override;
};

#endif
