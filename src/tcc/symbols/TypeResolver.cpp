#include "TypeResolver.h"

#include "FunctionLib.h"
#include "TypeLib.h"

std::any TypeResolver::modify(std::any data)
{
    if (data.type() != typeid(AsgNode*)) {
        return {};
    }
    auto* root = std::any_cast<AsgNode*>(data);
    root->accept(this);
    if (ok_) {
        return root;
    }
    return {};
}

std::any TypeResolver::visitStatementList(struct AsgStatementList* node)
{
    for (auto& statement : node->statements) {
        statement->accept(this);
    }
    return Type::invalid();
}

std::any TypeResolver::visitStructDefinition(struct AsgStructDefinition* node)
{
    return Type::invalid();
}

std::any TypeResolver::visitFunctionDefinition(struct AsgFunctionDefinition* node)
{
    node->body->accept(this);
    return Type::invalid();
}

std::any TypeResolver::visitVariableDefinition(struct AsgVariableDefinition* node)
{
    auto nodeType = TypeLibrary::inst().get(node->type);
    if (node->value) {
        auto valueType = std::any_cast<Type::Id>(node->value->accept(this));
        if (!Type::isSame(nodeType, valueType)) {
            ok_ = false;
        }
    }
    return Type::invalid();
}

std::any TypeResolver::visitReturn(struct AsgReturn* node)
{
    auto expected = node->function->type->returnType;
    auto real = std::any_cast<Type::Id>(node->value->accept(this));
    if (!Type::isSame(real, expected)) {
        ok_ = false;
    }
    return Type::invalid();
}

std::any TypeResolver::visitAssignment(struct AsgAssignment* node)
{
    auto valueType = std::any_cast<Type::Id>(node->value->accept(this));
    auto assignableType = std::any_cast<Type::Id>(node->assignable->accept(this));
    if (!Type::isSame(valueType, assignableType)) {
        ok_ = false;
        return Type::invalid();
    }
    node->exprType = valueType;
    return valueType;
}

std::any TypeResolver::visitConditional(struct AsgConditional* node)
{
    node->condition->accept(this);
    node->thenNode->accept(this);
    if (node->elseNode) {
        node->elseNode->accept(this);
    }
    return Type::invalid();
}

std::any TypeResolver::visitLoop(struct AsgLoop* node)
{
    node->condition->accept(this);
    node->body->accept(this);
    return Type::invalid();
}

std::any TypeResolver::visitComp(struct AsgComp* node)
{
    auto lhsType = std::any_cast<Type::Id>(node->lhs->accept(this));
    auto rhsType = std::any_cast<Type::Id>(node->rhs->accept(this));
    if (!Type::isSame(lhsType, rhsType)) {
        ok_ = false;
        return Type::invalid();
    }
    node->exprType = TypeLibrary::inst().get("int");
    return node->exprType;
}

std::any TypeResolver::visitAddSub(struct AsgAddSub* node)
{
    for (auto& expr : node->subexpressions) {
        if (std::any_cast<Type::Id>(expr.expression->accept(this)) != TypeLibrary::inst().get("int")) {
            ok_ = false;
            return Type::invalid();
        }
    }
    node->exprType = TypeLibrary::inst().get("int");
    return node->exprType;
}

std::any TypeResolver::visitMulDiv(struct AsgMulDiv* node)
{
    for (auto& expr : node->subexpressions) {
        if (std::any_cast<Type::Id>(expr.expression->accept(this)) != TypeLibrary::inst().get("int")) {
            ok_ = false;
            return Type::invalid();
        }
    }
    node->exprType = TypeLibrary::inst().get("int");
    return node->exprType;
}

std::any TypeResolver::visitIndexing(struct AsgIndexing* node)
{
    auto indexedType = std::any_cast<Type::Id>(node->indexed->accept(this));
    if (!indexedType || !indexedType->as<ArrayType>()) {
        ok_ = false;
        return Type::invalid();
    }
    for (auto& index : node->indexes) {
        auto indexType = std::any_cast<Type::Id>(index->accept(this));
        if (indexType != TypeLibrary::inst().get("int")) {
            ok_ = false;
            return Type::invalid();
        }
    }

    for (auto i = 0; i < node->indexes.size(); i++) {
        if (!indexedType || !indexedType->as<ArrayType>()) {
            ok_ = false;
            return Type::invalid();
        }
        indexedType = indexedType->as<ArrayType>()->getIndexed();
    }

    node->exprType = indexedType;
    return indexedType;
}

std::any TypeResolver::visitOpDeref(struct AsgOpDeref* node)
{
    auto t = std::any_cast<Type::Id>(node->expression->accept(this));

    for (auto i = 0; i < node->derefCount; i++) {
        if (!t || !t->as<PtrType>()) {
            ok_ = false;
            return Type::invalid();
        }
        t = t->as<PtrType>()->getDeref();
    }

    node->exprType = t;
    return t;
}

std::any TypeResolver::visitOpRef(struct AsgOpRef* node)
{
    auto t = std::any_cast<Type::Id>(node->value->accept(this));
    node->exprType = t->getRef();
    return node->exprType;
}

std::any TypeResolver::visitVariable(struct AsgVariable* node)
{
    const AsgStatementList* list = node->list;
    while (list) {
        if (list->localVariables.find(node->name) != list->localVariables.end()) {
            node->exprType = list->localVariables.at(node->name);
            return node->exprType;
        }
        list = list->list;
    }
    return Type::invalid();
}

std::any TypeResolver::visitCall(struct AsgCall* node)
{
    if (node->callee->parameters.size() != node->arguments.size()) {
        ok_ = false;
        return Type::invalid();
    }
    for (auto i = 0; i < node->arguments.size(); i++) {
        auto argT = std::any_cast<Type::Id>(node->arguments[i]->accept(this));
        if (!Type::isSame(node->callee->parameters[i], argT)) {
            ok_ = false;
            return Type::invalid();
        }
    }

    node->exprType = node->callee->returnType;
    return node->exprType;
}

std::any TypeResolver::visitIntLiteral(struct AsgIntLiteral* node)
{
    node->exprType = TypeLibrary::inst().get("int");
    return node->exprType;
}
