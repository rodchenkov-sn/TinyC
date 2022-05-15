#include "AstVisitor.h"

#include <numeric>

#include <spdlog/spdlog.h>

#include "asg/AsgNode.h"

std::any AstVisitor::modify(std::any data)
{
    if (data.type() != typeid(TinyCParser::TranslationUnitContext*)) {
        spdlog::critical("Unexpected data type passed to AstVisitor -- expected TranslationUnitContext*");
        return {};
    }
    auto* ctx = std::any_cast<TinyCParser::TranslationUnitContext*>(data);

    auto ret = visitTranslationUnit(ctx);

    return ret;
}

std::any AstVisitor::visitTranslationUnit(TinyCParser::TranslationUnitContext* ctx)
{
    auto node = std::make_unique<AsgStatementList>();
    for (auto* entity : ctx->entity()) {
        auto* function = std::any_cast<AsgNode*>(visit(entity));
        node->statements.emplace_back(function);
    }
    return (AsgNode*)node.release();
}

std::any AstVisitor::visitStruct(TinyCParser::StructContext* ctx)
{
    auto node = std::make_unique<AsgStructDefinition>();
    node->refLine = ctx->start->getLine();
    node->name = ctx->IDENTIFIER()->getText();
    for (auto* field : ctx->structField()) {
        AsgStructDefinition::Field f;
        const auto& indexes = field->constantIndexing();
        f.type = field->type()->typeName()->getText()
               + std::string(field->type()->ASTERISK().size(), '*')
               + std::accumulate(
                     indexes.begin(),
                     indexes.end(),
                     std::string{},
                     [](const std::string& accum, TinyCParser::ConstantIndexingContext* indexing) {
                         return accum + indexing->getText();
                     });
        f.name = field->IDENTIFIER()->getText();
        f.refLine = field->type()->start->getLine();
        node->fields.push_back(f);
    }
    return (AsgNode*)node.release();
}

std::any AstVisitor::visitFunction(TinyCParser::FunctionContext* ctx)
{
    auto node = std::make_unique<AsgFunctionDefinition>();
    node->refLine = ctx->start->getLine();
    node->returnType = ctx->type()->typeName()->getText()
                     + std::string(ctx->type()->ASTERISK().size(), '*');
    node->name = ctx->functionName()->getText();

    if (auto* params = ctx->parameters()) {
        for (auto* paramCtx : params->parameter()) {
            const auto& indexes = paramCtx->constantIndexing();

            AsgFunctionDefinition::Parameter p;
            p.type = paramCtx->type()->typeName()->getText()
                   + std::string(paramCtx->type()->ASTERISK().size(), '*')
                   + std::accumulate(
                         indexes.begin(),
                         indexes.end(),
                         std::string{},
                         [](const std::string& accum, TinyCParser::ConstantIndexingContext* indexing) {
                             return accum + indexing->getText();
                         });
            p.name = paramCtx->variableName()->getText();

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
    node->refLine = ctx->start->getLine();

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
    if (ctx->returnStatement()) {
        return visitReturnStatement(ctx->returnStatement());
    }
    if (ctx->ifStatement()) {
        return visitIfStatement(ctx->ifStatement());
    }
    if (ctx->forStatement()) {
        return visitForStatement(ctx->forStatement());
    }
    if (ctx->whileStatement()) {
        return visitWhileStatement(ctx->whileStatement());
    }
    return visitStatements(ctx->statements());
}

std::any AstVisitor::visitIfStatement(TinyCParser::IfStatementContext* ctx)
{
    auto node = std::make_unique<AsgConditional>();
    node->refLine = ctx->start->getLine();
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

std::any AstVisitor::visitVariableDecl(TinyCParser::VariableDeclContext* ctx)
{
    auto node = std::make_unique<AsgVariableDefinition>();
    node->refLine = ctx->start->getLine();
    const auto& indexes = ctx->constantIndexing();

    node->type = ctx->type()->typeName()->getText()
               + std::string(ctx->type()->ASTERISK().size(), '*')
               + std::accumulate(
                     indexes.begin(),
                     indexes.end(),
                     std::string{},
                     [](const std::string& accum, TinyCParser::ConstantIndexingContext* indexing) {
                         return accum + indexing->getText();
                     });

    node->name = ctx->variableName()->getText();

    if (ctx->expression()) {
        auto* e = std::any_cast<AsgNode*>(visit(ctx->expression()));
        node->value.reset(e);
    }

    return (AsgNode*)node.release();
}

std::any AstVisitor::visitAssignment(TinyCParser::AssignmentContext* ctx)
{
    auto node = std::make_unique<AsgAssignment>();
    node->refLine = ctx->start->getLine();
    node->assignable.reset(std::any_cast<AsgNode*>(visit(ctx->operandDereference())));

    auto* e = std::any_cast<AsgNode*>(visit(ctx->expression()));
    node->value.reset(e);

    return (AsgNode*)node.release();
}

std::any AstVisitor::visitReturnStatement(TinyCParser::ReturnStatementContext* ctx)
{
    auto node = std::make_unique<AsgReturn>();
    node->refLine = ctx->start->getLine();
    if (ctx->expression()) {
        auto* e = std::any_cast<AsgNode*>(visit(ctx->expression()));
        node->value.reset(e);
    }

    return node.release();
}

std::any AstVisitor::visitWhileStatement(TinyCParser::WhileStatementContext* ctx)
{
    auto node = std::make_unique<AsgLoop>();

    node->condition.reset(std::any_cast<AsgNode*>(visit(ctx->expression())));
    node->body.reset(std::any_cast<AsgNode*>(visit(ctx->statement())));

    return (AsgNode*)node.release();
}

std::any AstVisitor::visitForStatement(TinyCParser::ForStatementContext* ctx)
{
    auto node = std::make_unique<AsgStatementList>();
    node->statements.emplace_back(std::any_cast<AsgNode*>(visit(ctx->expression(0))));
    {
        auto loopNode = std::make_unique<AsgLoop>();
        loopNode->condition.reset(std::any_cast<AsgNode*>(visit(ctx->expression(1))));
        {
            auto loopBody = std::make_unique<AsgStatementList>();
            loopBody->statements.emplace_back(std::any_cast<AsgNode*>(visit(ctx->statement())));
            loopBody->statements.emplace_back(std::any_cast<AsgNode*>(visit(ctx->expression(2))));
            loopNode->body = std::move(loopBody);
        }
        node->statements.emplace_back(std::move(loopNode));
    }
    return (AsgNode*)node.release();
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
    node->refLine = ctx->start->getLine();
    node->lhs.reset(lhs);
    node->rhs.reset(rhs);
    node->op = op;

    return (AsgNode*)node.release();
}

std::any AstVisitor::visitAddSubExpr(TinyCParser::AddSubExprContext* ctx)
{
    if (ctx->mulDivExpr().size() == 1) {
        return visit(ctx->mulDivExpr(0));
    }

    auto node = std::make_unique<AsgAddSub>();
    node->refLine = ctx->start->getLine();
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

std::any AstVisitor::visitMulDivExpr(TinyCParser::MulDivExprContext* ctx)
{
    if (ctx->operandDereference().size() == 1) {
        return visit(ctx->operandDereference(0));
    }

    auto node = std::make_unique<AsgMulDiv>();
    node->refLine = ctx->start->getLine();
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

std::any AstVisitor::visitIndexedOperand(TinyCParser::IndexedOperandContext* ctx)
{
    if (ctx->indexing().empty()) {
        return visit(ctx->operand());
    }

    auto node = std::make_unique<AsgIndexing>();
    node->refLine = ctx->start->getLine();
    node->indexed.reset(std::any_cast<AsgNode*>(visit(ctx->operand())));

    for (auto* indexCtx : ctx->indexing()) {
        node->indexes.emplace_back(std::any_cast<AsgNode*>(visit(indexCtx->expression())));
    }

    return (AsgNode*)node.release();
}

std::any AstVisitor::visitOperandDereference(TinyCParser::OperandDereferenceContext* ctx)
{
    if (ctx->ASTERISK().empty()) {
        return visit(ctx->fieldAccess());
    }

    auto node = std::make_unique<AsgOpDeref>();
    node->refLine = ctx->start->getLine();
    node->derefCount = ctx->ASTERISK().size();
    node->expression.reset(std::any_cast<AsgNode*>(visit(ctx->fieldAccess())));
    return (AsgNode*)node.release();
}

std::any AstVisitor::visitFieldAccess(TinyCParser::FieldAccessContext* ctx)
{
    if (ctx->fieldAccessOp().empty()) {
        return visit(ctx->indexedOperand());
    }

    auto accessed = std::any_cast<AsgNode*>(visit(ctx->indexedOperand()));
    for (auto* access : ctx->fieldAccessOp()) {
        if (access->ARROW()) {
            auto derefNode = std::make_unique<AsgOpDeref>();
            derefNode->refLine = access->getStart()->getLine();
            derefNode->derefCount = 1;
            derefNode->expression.reset(accessed);
            accessed = derefNode.release();
        }

        auto accNode = std::make_unique<AsgFieldAccess>();
        accNode->refLine = access->getStart()->getLine();
        accNode->field = access->IDENTIFIER()->getText();
        accNode->accessed.reset(accessed);

        if (access->indexing().empty()) {
            accessed = accNode.release();
            continue;
        }

        auto indexingNode = std::make_unique<AsgIndexing>();
        indexingNode->refLine = access->getStart()->getLine();
        indexingNode->indexed = std::move(accNode);

        for (auto* index : access->indexing()) {
            indexingNode->indexes.emplace_back(std::any_cast<AsgNode*>(visit(index->expression())));
        }

        accessed = indexingNode.release();
    }

    return accessed;
}

std::any AstVisitor::visitValueReference(TinyCParser::ValueReferenceContext* ctx)
{
    if (!ctx->AMPERSAND()) {
        return visit(ctx->referenceableValue());
    }

    auto node = std::make_unique<AsgOpRef>();
    node->refLine = ctx->start->getLine();
    node->value.reset(std::any_cast<AsgNode*>(visit(ctx->referenceableValue())));
    return (AsgNode*)node.release();
}

std::any AstVisitor::visitCallExpr(TinyCParser::CallExprContext* ctx)
{
    auto node = std::make_unique<AsgCall>();

    node->functionName = ctx->functionName()->getText();
    node->refLine = ctx->start->getLine();
    if (auto* argumentsCtx = ctx->arguments()) {
        for (auto* argumentCtx : argumentsCtx->argument()) {
            auto* argument = std::any_cast<AsgNode*>(visit(argumentCtx->expression()));
            node->arguments.emplace_back(argument);
        }
    }

    return (AsgNode*)node.release();
}

std::any AstVisitor::visitLiteral(TinyCParser::LiteralContext* ctx)
{
    auto node = std::make_unique<AsgIntLiteral>();
    node->refLine = ctx->start->getLine();
    node->value = std::stoi(ctx->getText());

    return (AsgNode*)node.release();
}

std::any AstVisitor::visitVariableName(TinyCParser::VariableNameContext* ctx)
{
    auto node = std::make_unique<AsgVariable>();
    node->refLine = ctx->start->getLine();
    node->name = ctx->getText();

    return (AsgNode*)node.release();
}
