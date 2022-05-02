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
    node->returnType = ctx->type()->typeName()->getText()
                     + std::string(ctx->type()->ASTERISK().size(), '*');
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

        auto stmtNode = visit(statement);

        if (stmtNode.type() == typeid(AsgReturn*)) {
            node->statements.emplace_back(std::any_cast<AsgReturn*>(stmtNode));
            break;
        }
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
    if (ctx->variableDecl()) {
        return visitVariableDecl(ctx->variableDecl());
    }
    if (ctx->ifStatement()) {
        return visitIfStatement(ctx->ifStatement());
    }
    return visitStatements(ctx->statements());
}


std::any AstVisitor::visitIfStatement(TinyCParser::IfStatementContext* ctx)
{
    auto node = std::make_unique<AsgConditional>();

    node->condition.reset(std::any_cast<AsgNode*>(visit(ctx->expression())));

    auto thenNode = visit(ctx->statement(0));
    if (thenNode.type() == typeid(AsgNode*)) {
        node->thenNode.reset(std::any_cast<AsgNode*>(thenNode));
    } else {
        node->thenNode.reset(std::any_cast<AsgReturn*>(thenNode));
    }

    if (ctx->statement().size() == 2) {
        auto elseNode = visit(ctx->statement(1));
        if (elseNode.type() == typeid(AsgNode*)) {
            node->elseNode.reset(std::any_cast<AsgNode*>(elseNode));
        } else {
            node->elseNode.reset(std::any_cast<AsgReturn*>(elseNode));
        }
    }

    return (AsgNode*)node.release();
}


std::any AstVisitor::visitVariableDecl(TinyCParser::VariableDeclContext *ctx)
{
    auto node = std::make_unique<AsgVariableDefinition>();

    node->type = ctx->type()->typeName()->getText()
               + std::string(ctx->type()->ASTERISK().size(), '*');
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

    if (ctx->assignable()->ASTERISK().empty()) {
        node->name = ctx->assignable()->variableName()->getText();
    } else {
        auto deref = std::make_unique<AsgOpDeref>();
        deref->derefCount = ctx->assignable()->ASTERISK().size() - 1;
        auto varNode = std::make_unique<AsgVariable>();
        varNode->name = ctx->assignable()->variableName()->getText();
        deref->expression = std::move(varNode);
        node->assignable = std::move(deref);
    }

    auto* e = std::any_cast<AsgNode*>(visit(ctx->expression()));
    node->value.reset(e);

    return (AsgNode*)node.release();
}

std::any AstVisitor::visitReturnStatement(TinyCParser::ReturnStatementContext *ctx)
{
    auto node = std::make_unique<AsgReturn>();

    auto* e = std::any_cast<AsgNode*>(visit(ctx->expression()));
    node->value.reset(e);

    return node.release();
}


std::any AstVisitor::visitCompExpression(TinyCParser::CompExpressionContext* ctx)
{
    if (ctx->addSubExpr().size() == 1) {
        return visitAddSubExpr(ctx->addSubExpr(0));
    }

    auto* lhs = std::any_cast<AsgNode*>(visitAddSubExpr(ctx->addSubExpr(0)));
    auto* rhs = std::any_cast<AsgNode*>(visitAddSubExpr(ctx->addSubExpr(1)));
    AsgComp::Operator op;
    if (ctx->EQUALEQUAL()) {
        op = AsgComp::Operator::Equals;
    } else if (ctx->NOTEQUAL()) {
        op = AsgComp::Operator::NotEquals;
    } else if (ctx->LESS()) {
        op = AsgComp::Operator::Less;
    } else if (ctx->LESSEQUAL()) {
        op = AsgComp::Operator::LessEquals;
    } else if (ctx->GREATER()) {
        op = AsgComp::Operator::Greater;
    } else {
        op = AsgComp::Operator::GreaterEquals;
    }

    auto node = std::make_unique<AsgComp>();

    node->lhs = lhs;
    node->rhs = rhs;
    node->op = op;

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
    if (ctx->operandDereference().size() == 1) {
        return visit(ctx->operandDereference(0));
    }

    auto node = std::make_unique<AsgMulDiv>();

    for (auto i = 0; i < ctx->operandDereference().size(); i++) {
        auto* e = std::any_cast<AsgNode*>(visit(ctx->operandDereference(i)));
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


std::any AstVisitor::visitOperandDereference(TinyCParser::OperandDereferenceContext* ctx)
{
    if (ctx->ASTERISK().empty()) {
        return visit(ctx->operand());
    }

    auto node = std::make_unique<AsgOpDeref>();
    node->derefCount = ctx->ASTERISK().size();
    node->expression.reset(std::any_cast<AsgNode*>(visit(ctx->operand())));
    return (AsgNode*)node.release();
}


std::any AstVisitor::visitValueReference(TinyCParser::ValueReferenceContext* ctx)
{
    if (!ctx->AMPERSAND()) {
        return visit(ctx->referenceableValue());
    }

    auto node = std::make_unique<AsgOpRef>();
    node->value.reset(std::any_cast<AsgNode*>(visit(ctx->referenceableValue())));
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
