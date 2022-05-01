#include "AstVisitor.h"

#include "asg/AsgNode.h"


std::any AstVisitor::visitTranslationUnit(TinyCParser::TranslationUnitContext *ctx)
{
    auto node = std::make_unique<AsgStatementList>();
    for (auto* functionCtx : ctx->function()) {
        auto* function = std::any_cast<AsgNode*>(visit(functionCtx));
        node->statements.emplace_back(function);
    }
    return (AsgNode*)node.release();
}


std::any AstVisitor::visitFunction(TinyCParser::FunctionContext *ctx)
{
    auto node = std::make_unique<AsgFunctionDefinition>();
    node->returnType = ctx->type()->getText();
    node->name = ctx->functionName()->getText();

    if (auto* params = ctx->parameters()) {
        for (auto i = 0; i < params->type().size(); i++) {
            AsgFunctionDefinition::Parameter p{
                params->type(i)->getText(),
                params->variableName(i)->getText()
            };
            node->parameters.push_back(p);
        }
    }

    if (ctx->statements()) {
        auto* body = std::any_cast<AsgNode*>(visit(ctx->statements()));
        node->body.reset(body);
    }

    return (AsgNode*)node.release();
}


std::any AstVisitor::visitStatements(TinyCParser::StatementsContext* ctx)
{
    auto node = std::make_unique<AsgStatementList>();

    for (auto* statement : ctx->statement()) {
        auto* s = std::any_cast<AsgNode*>(visit(statement));
        node->statements.emplace_back(s);
    }

    return (AsgNode*)node.release();
}


std::any AstVisitor::visitStatement(TinyCParser::StatementContext* ctx)
{
    if (ctx->expression()) {
        return visitExpression(ctx->expression());
    }
    if (ctx->assignment()) {
        return visitAssignment(ctx->assignment());
    }
    if (ctx->returnStatement()) {
        return visitReturnStatement(ctx->returnStatement());
    }
    return visitVariableDecl(ctx->variableDecl());
}


std::any AstVisitor::visitVariableDecl(TinyCParser::VariableDeclContext *ctx)
{
    auto node = std::make_unique<AsgVariableDefinition>();

    node->type = ctx->type()->getText();
    node->name = ctx->variableName()->getText();

    if (ctx->expression()) {
        auto* e = std::any_cast<AsgNode*>(visit(ctx->expression()));
        node->value.reset(e);
    }

    return (AsgNode*)node.release();
}

std::any AstVisitor::visitAssignment(TinyCParser::AssignmentContext *ctx)
{
    auto node = std::make_unique<AsgAssignment>();

    node->name = ctx->variableName()->getText();

    auto* e = std::any_cast<AsgNode*>(visit(ctx->expression()));
    node->value.reset(e);

    return (AsgNode*)node.release();
}

std::any AstVisitor::visitReturnStatement(TinyCParser::ReturnStatementContext *ctx)
{
    auto node = std::make_unique<AsgReturn>();

    auto* e = std::any_cast<AsgNode*>(visit(ctx->expression()));
    node->value.reset(e);

    return (AsgNode*)node.release();
}


std::any AstVisitor::visitAddSubExpr(TinyCParser::AddSubExprContext *ctx)
{
    if (ctx->mulDivExpr().size() == 1) {
        return visit(ctx->mulDivExpr(0));
    }

    auto node = std::make_unique<AsgAddSub>();

    for (auto i = 0; i < ctx->mulDivExpr().size(); i++) {
        auto* e = std::any_cast<AsgNode*>(visit(ctx->mulDivExpr(i)));
        AsgAddSub::Operator op = AsgAddSub::Operator::Add;
        if (i != 0 && ctx->MINUS(i - 1)) {
            op = AsgAddSub::Operator::Sub;
        }

        AsgAddSub::Subexpression& se = node->subexpressions.emplace_back();
        {
            se.leadingOp = op;
            se.expression.reset(e);
        }
    }

    return (AsgNode*)node.release();
}

std::any AstVisitor::visitMulDivExpr(TinyCParser::MulDivExprContext *ctx)
{
    if (ctx->operand().size() == 1) {
        return visit(ctx->operand(0));
    }

    auto node = std::make_unique<AsgMulDiv>();

    for (auto i = 0; i < ctx->operand().size(); i++) {
        auto* e = std::any_cast<AsgNode*>(visit(ctx->operand(i)));
        AsgMulDiv::Operator op = AsgMulDiv::Operator::Mul;
        if (i != 0 && ctx->SLASH(i - 1)) {
            op = AsgMulDiv::Operator::Div;
        }

        AsgMulDiv::Subexpression& se = node->subexpressions.emplace_back();
        {
            se.leadingOp = op;
            se.expression.reset(e);
        }
    }

    return (AsgNode*)node.release();
}


std::any AstVisitor::visitCallExpr(TinyCParser::CallExprContext *ctx)
{
    auto node = std::make_unique<AsgCall>();

    node->functionName = ctx->functionName()->getText();

    if (auto* argumentsCtx = ctx->arguments()) {
        for (auto* argumentCtx : argumentsCtx->argument()) {
            auto* argument = std::any_cast<AsgNode*>(visit(argumentCtx->expression()));
            node->arguments.emplace_back(argument);
        }
    }

    return (AsgNode*)node.release();
}


std::any AstVisitor::visitLiteral(TinyCParser::LiteralContext *ctx)
{
    auto node = std::make_unique<AsgIntLiteral>();

    node->value = std::stoi(ctx->getText());

    return (AsgNode*)node.release();
}


std::any AstVisitor::visitVariableName(TinyCParser::VariableNameContext *ctx)
{
    auto node = std::make_unique<AsgVariable>();

    node->name = ctx->getText();

    return (AsgNode*)node.release();
}
