#ifndef TINYC_ASTVISITOR_H
#define TINYC_ASTVISITOR_H

#include "TinyCBaseVisitor.h"


class AstVisitor : public TinyCBaseVisitor {
public:
    std::any visitTranslationUnit(TinyCParser::TranslationUnitContext *ctx) override;
    std::any visitFunction(TinyCParser::FunctionContext *ctx) override;
    std::any visitStatements(TinyCParser::StatementsContext *ctx) override;
    std::any visitStatement(TinyCParser::StatementContext *ctx) override;
    std::any visitVariableDecl(TinyCParser::VariableDeclContext *ctx) override;
    std::any visitAssignment(TinyCParser::AssignmentContext *ctx) override;
    std::any visitReturnStatement(TinyCParser::ReturnStatementContext *ctx) override;
    std::any visitAddSubExpr(TinyCParser::AddSubExprContext *ctx) override;
    std::any visitMulDivExpr(TinyCParser::MulDivExprContext *ctx) override;
    std::any visitCallExpr(TinyCParser::CallExprContext *ctx) override;
    std::any visitLiteral(TinyCParser::LiteralContext *ctx) override;
    std::any visitVariableName(TinyCParser::VariableNameContext *ctx) override;

private:
    int first_free_tmp_reg_ = 0;
};


#endif // TINYC_ASTVISITOR_H
