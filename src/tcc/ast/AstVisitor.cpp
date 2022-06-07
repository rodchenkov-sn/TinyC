#include "AstVisitor.h"

#include <numeric>

#include "asg/AsgNode.h"
#include "log/Logging.h"

std::any AstVisitor::modify(std::any data)
{
    if (data.type() != typeid(TinyCParser::TranslationUnitContext*)) {
        TC_LOG_CRITICAL("Unexpected data type passed to AstVisitor -- expected TranslationUnitContext*");
        return {};
    }
    auto* ctx = std::any_cast<TinyCParser::TranslationUnitContext*>(data);

    auto ret = visitTranslationUnit(ctx);

    return ret.as<AsgNode*>();
}

antlrcpp::Any AstVisitor::visitTranslationUnit(TinyCParser::TranslationUnitContext* ctx)
{
    auto node = std::make_unique<AsgStatementList>();
    for (auto* entity : ctx->entity()) {
        auto* function = visit(entity).as<AsgNode*>();
        node->statements.emplace_back(function);
    }
    return (AsgNode*)node.release();
}

antlrcpp::Any AstVisitor::visitStructDef(TinyCParser::StructDefContext* ctx)
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

antlrcpp::Any AstVisitor::visitFunctionDef(TinyCParser::FunctionDefContext* ctx)
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
        auto* body = visit(ctx->statements()).as<AsgNode*>();
        node->body.reset(body);
    }

    return (AsgNode*)node.release();
}

antlrcpp::Any AstVisitor::visitStatements(TinyCParser::StatementsContext* ctx)
{
    auto node = std::make_unique<AsgStatementList>();
    node->refLine = ctx->start->getLine();

    for (auto* statement : ctx->statement()) {

        auto stmtNode = visit(statement);

        if (stmtNode.is<AsgReturn*>()) {
            node->statements.emplace_back(stmtNode.as<AsgReturn*>());
            break;
        }
        auto* s = visit(statement).as<AsgNode*>();
        node->statements.emplace_back(s);
    }

    return (AsgNode*)node.release();
}

antlrcpp::Any AstVisitor::visitStatement(TinyCParser::StatementContext* ctx)
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

antlrcpp::Any AstVisitor::visitIfStatement(TinyCParser::IfStatementContext* ctx)
{
    auto node = std::make_unique<AsgConditional>();
    node->refLine = ctx->start->getLine();
    node->condition.reset(visit(ctx->expression()).as<AsgNode*>());

    auto thenNode = visit(ctx->statement(0));
    if (thenNode.is<AsgNode*>()) {
        node->thenNode.reset(thenNode.as<AsgNode*>());
    } else {
        node->thenNode.reset(thenNode.as<AsgReturn*>());
    }

    if (ctx->statement().size() == 2) {
        auto elseNode = visit(ctx->statement(1));
        if (elseNode.is<AsgNode*>()) {
            node->elseNode.reset(elseNode.as<AsgNode*>());
        } else {
            node->elseNode.reset(elseNode.as<AsgReturn*>());
        }
    }

    return (AsgNode*)node.release();
}

antlrcpp::Any AstVisitor::visitVariableDecl(TinyCParser::VariableDeclContext* ctx)
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
        auto* e = visit(ctx->expression()).as<AsgNode*>();
        node->value.reset(e);
    }

    return (AsgNode*)node.release();
}

antlrcpp::Any AstVisitor::visitAssignment(TinyCParser::AssignmentContext* ctx)
{
    auto node = std::make_unique<AsgAssignment>();
    node->refLine = ctx->start->getLine();
    node->assignable.reset(visit(ctx->operandDereference()).as<AsgNode*>());

    auto* e = visit(ctx->expression()).as<AsgNode*>();
    node->value.reset(e);

    return (AsgNode*)node.release();
}

antlrcpp::Any AstVisitor::visitReturnStatement(TinyCParser::ReturnStatementContext* ctx)
{
    auto node = std::make_unique<AsgReturn>();
    node->refLine = ctx->start->getLine();
    if (ctx->expression()) {
        auto* e = visit(ctx->expression()).as<AsgNode*>();
        node->value.reset(e);
    }

    return node.release();
}

antlrcpp::Any AstVisitor::visitWhileStatement(TinyCParser::WhileStatementContext* ctx)
{
    auto node = std::make_unique<AsgLoop>();

    node->condition.reset(visit(ctx->expression()).as<AsgNode*>());
    node->body.reset(visit(ctx->statement()).as<AsgNode*>());

    return (AsgNode*)node.release();
}

antlrcpp::Any AstVisitor::visitForStatement(TinyCParser::ForStatementContext* ctx)
{
    auto node = std::make_unique<AsgStatementList>();
    node->statements.emplace_back(visit(ctx->expression(0)).as<AsgNode*>());
    {
        auto loopNode = std::make_unique<AsgLoop>();
        loopNode->condition.reset(visit(ctx->expression(1)).as<AsgNode*>());
        {
            auto loopBody = std::make_unique<AsgStatementList>();
            loopBody->statements.emplace_back(visit(ctx->statement()).as<AsgNode*>());
            loopBody->statements.emplace_back(visit(ctx->expression(2)).as<AsgNode*>());
            loopNode->body = std::move(loopBody);
        }
        node->statements.emplace_back(std::move(loopNode));
    }
    return (AsgNode*)node.release();
}

antlrcpp::Any AstVisitor::visitCompExpression(TinyCParser::CompExpressionContext* ctx)
{
    if (ctx->addSubExpr().size() == 1) {
        return visitAddSubExpr(ctx->addSubExpr(0));
    }

    auto* lhs = visitAddSubExpr(ctx->addSubExpr(0)).as<AsgNode*>();
    auto* rhs = visitAddSubExpr(ctx->addSubExpr(1)).as<AsgNode*>();
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

antlrcpp::Any AstVisitor::visitAddSubExpr(TinyCParser::AddSubExprContext* ctx)
{
    if (ctx->mulDivExpr().size() == 1) {
        return visit(ctx->mulDivExpr(0));
    }

    auto node = std::make_unique<AsgAddSub>();
    node->refLine = ctx->start->getLine();
    for (auto i = 0; i < ctx->mulDivExpr().size(); i++) {
        auto* e = visit(ctx->mulDivExpr(i)).as<AsgNode*>();
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

antlrcpp::Any AstVisitor::visitMulDivExpr(TinyCParser::MulDivExprContext* ctx)
{
    if (ctx->operandDereference().size() == 1) {
        return visit(ctx->operandDereference(0));
    }

    auto node = std::make_unique<AsgMulDiv>();
    node->refLine = ctx->start->getLine();
    for (auto i = 0; i < ctx->operandDereference().size(); i++) {
        auto* e = visit(ctx->operandDereference(i)).as<AsgNode*>();
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

antlrcpp::Any AstVisitor::visitIndexedOperand(TinyCParser::IndexedOperandContext* ctx)
{
    if (ctx->indexing().empty()) {
        return visit(ctx->operand());
    }

    auto node = std::make_unique<AsgIndexing>();
    node->refLine = ctx->start->getLine();
    node->indexed.reset(visit(ctx->operand()).as<AsgNode*>());

    for (auto* indexCtx : ctx->indexing()) {
        node->indexes.emplace_back(visit(indexCtx->expression()).as<AsgNode*>());
    }

    return (AsgNode*)node.release();
}

antlrcpp::Any AstVisitor::visitOperandDereference(TinyCParser::OperandDereferenceContext* ctx)
{
    if (ctx->ASTERISK().empty()) {
        return visit(ctx->fieldAccess());
    }

    auto node = std::make_unique<AsgOpDeref>();
    node->refLine = ctx->start->getLine();
    node->derefCount = ctx->ASTERISK().size();
    node->expression.reset(visit(ctx->fieldAccess()).as<AsgNode*>());
    return (AsgNode*)node.release();
}

antlrcpp::Any AstVisitor::visitFieldAccess(TinyCParser::FieldAccessContext* ctx)
{
    if (ctx->fieldAccessOp().empty()) {
        return visit(ctx->indexedOperand());
    }

    auto accessed = visit(ctx->indexedOperand()).as<AsgNode*>();
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
            indexingNode->indexes.emplace_back(visit(index->expression()).as<AsgNode*>());
        }

        accessed = indexingNode.release();
    }

    return accessed;
}

antlrcpp::Any AstVisitor::visitValueReference(TinyCParser::ValueReferenceContext* ctx)
{
    if (!ctx->AMPERSAND()) {
        return visit(ctx->referenceableValue());
    }

    auto node = std::make_unique<AsgOpRef>();
    node->refLine = ctx->start->getLine();
    node->value.reset(visit(ctx->referenceableValue()).as<AsgNode*>());
    return (AsgNode*)node.release();
}

antlrcpp::Any AstVisitor::visitCallExpr(TinyCParser::CallExprContext* ctx)
{
    auto node = std::make_unique<AsgCall>();

    node->functionName = ctx->functionName()->getText();
    node->refLine = ctx->start->getLine();
    if (auto* argumentsCtx = ctx->arguments()) {
        for (auto* argumentCtx : argumentsCtx->argument()) {
            auto* argument = visit(argumentCtx->expression()).as<AsgNode*>();
            node->arguments.emplace_back(argument);
        }
    }

    return (AsgNode*)node.release();
}

antlrcpp::Any AstVisitor::visitLiteral(TinyCParser::LiteralContext* ctx)
{
    auto node = std::make_unique<AsgIntLiteral>();
    node->refLine = ctx->start->getLine();
    node->value = std::stoi(ctx->getText());

    return (AsgNode*)node.release();
}

antlrcpp::Any AstVisitor::visitVariableName(TinyCParser::VariableNameContext* ctx)
{
    auto node = std::make_unique<AsgVariable>();
    node->refLine = ctx->start->getLine();
    node->name = ctx->getText();

    return (AsgNode*)node.release();
}
