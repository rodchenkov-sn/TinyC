#include "TypeResolver.h"

#include <spdlog/spdlog.h>

#include "FunctionLib.h"
#include "TypeLib.h"

std::any TypeResolver::modify(std::any data)
{
    if (data.type() != typeid(AsgNode*)) {
        spdlog::critical("Unexpected data type passed to TypeResolver -- expected AsgNode*");
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
            if (nodeType && valueType) {
                spdlog::error(
                    "at line {} -- invalid type conversion in assignment: {} -> {}",
                    node->refLine,
                    valueType->toString(),
                    nodeType->toString());
            }
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
        if (real && expected) {
            spdlog::error(
                "at line {} -- invalid return type: expected {}, got {}",
                node->refLine,
                expected->toString(),
                real->toString());
        }
        ok_ = false;
    }
    return Type::invalid();
}

std::any TypeResolver::visitAssignment(struct AsgAssignment* node)
{
    auto valueType = std::any_cast<Type::Id>(node->value->accept(this));
    auto assignableType = std::any_cast<Type::Id>(node->assignable->accept(this));
    if (!Type::isSame(valueType, assignableType)) {
        if (valueType && assignableType) {
            spdlog::error(
                "at line {} -- invalid type conversion in assignment: {} -> {}",
                node->refLine,
                valueType->toString(),
                assignableType->toString());
        }
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
        if (rhsType && lhsType) {
            spdlog::error(
                "at line {} -- comparison between values of different types: {} vs {}",
                node->refLine,
                lhsType->toString(),
                rhsType->toString());
        }
        ok_ = false;
        return Type::invalid();
    }
    node->exprType = TypeLibrary::inst().get("int");
    return node->exprType;
}

std::any TypeResolver::visitAddSub(struct AsgAddSub* node)
{
    for (auto& expr : node->subexpressions) {
        if (auto t = std::any_cast<Type::Id>(expr.expression->accept(this)); t != TypeLibrary::inst().get("int")) {
            if (t) {
                spdlog::error("at line {} -- invalid arithmetic with type {}", node->refLine, t->toString());
            }
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
        if (auto t = std::any_cast<Type::Id>(expr.expression->accept(this)); t != TypeLibrary::inst().get("int")) {
            if (t) {
                spdlog::error("at line {} -- invalid arithmetic with type {}", node->refLine, t->toString());
            }
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
        if (indexedType) {
            spdlog::error("at line {} -- invalid indexing of type {}", node->refLine, indexedType->toString());
        }
        ok_ = false;
        return Type::invalid();
    }
    for (auto& index : node->indexes) {
        auto indexType = std::any_cast<Type::Id>(index->accept(this));
        if (indexType != TypeLibrary::inst().get("int")) {
            if (indexType) {
                spdlog::error(
                    "at line {} -- invalid index type: expected: int, got: {}",
                    index->refLine,
                    indexedType->toString());
            }
            ok_ = false;
            return Type::invalid();
        }
    }

    auto prevType = indexedType;

    for (auto& i : node->indexes) {
        if (!indexedType || !indexedType->as<ArrayType>()) {
            if (prevType) {
                spdlog::error("at line {} -- invalid indexing: cant index {}", i->refLine, prevType->toString());
            }
            ok_ = false;
            return Type::invalid();
        }
        prevType = indexedType;
        indexedType = indexedType->as<ArrayType>()->getIndexed();
    }

    node->exprType = indexedType;
    return indexedType;
}

std::any TypeResolver::visitOpDeref(struct AsgOpDeref* node)
{
    auto t = std::any_cast<Type::Id>(node->expression->accept(this));

    auto prevT = t;

    for (auto i = 0; i < node->derefCount; i++) {
        if (!t || !t->as<PtrType>()) {
            if (prevT) {
                spdlog::error("at line {} -- cant deref {}", node->refLine, prevT->toString());
            }
            ok_ = false;
            return Type::invalid();
        }
        prevT = t;
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
        spdlog::error(
            "at line {} -- invalid {} call arg count: expected: {}, got: {}",
            node->refLine,
            node->functionName,
            node->callee->parameters.size(),
            node->arguments.size());
        ok_ = false;
        return Type::invalid();
    }
    for (auto i = 0; i < node->arguments.size(); i++) {
        auto argT = std::any_cast<Type::Id>(node->arguments[i]->accept(this));
        if (!Type::isSame(node->callee->parameters[i], argT)) {
            if (argT) {
                spdlog::error(
                    "at line {} -- invalid arg #{} type: expected: {}, got: {}",
                    node->arguments[i]->refLine,
                    i + 1,
                    node->callee->parameters[i]->toString(),
                    argT->toString());
            }
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
