#include "TypeResolver.h"

#include "TypeLib.h"

struct VisUpdater {
    VisUpdater(AsgNode* n, std::deque<AsgNode*>& ns)
        : ns(ns)
    {
        ns.push_back(n);
    }

    ~VisUpdater()
    {
        ns.pop_back();
    }

    std::deque<AsgNode*>& ns;
};

#define UPDATE_VIS(n, ns)           \
    VisUpdater visUpdater##__LINE__ \
    {                               \
        (n), (ns)                   \
    }

static std::string getTmpRetName(const std::string& funName)
{
    return "." + funName + "_ret";
}

static std::string getTmpParamName(const std::string& origName)
{
    return "." + origName + "_par";
}

std::any TypeResolver::modify(std::any data)
{
    if (data.type() != typeid(AsgNode*)) {
        TC_LOG_CRITICAL("Unexpected data type passed to TypeResolver -- expected AsgNode*");
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
    UPDATE_VIS(node, nodes_);

    std::vector<AsgNode*> toVisit;

    std::transform(
        node->statements.begin(),
        node->statements.end(),
        std::back_inserter(toVisit),
        [](const std::unique_ptr<AsgNode>& n) { return n.get(); });

    for (auto* statement : toVisit) {
        statement->accept(this);
    }
    return LRValue{Type::invalid()};
}

std::any TypeResolver::visitStructDefinition(struct AsgStructDefinition* node)
{
    UPDATE_VIS(node, nodes_);
    return LRValue{Type::invalid()};
}

std::any TypeResolver::visitFunctionDefinition(struct AsgFunctionDefinition* node)
{
    UPDATE_VIS(node, nodes_);
    for (auto i = 0; i < node->parameters.size(); i++) {
        if (node->type->parameters[i]->as<StructType>()) {
            auto origName = node->parameters[i].name;
            auto origTypeStr = node->parameters[i].type;

            node->type->parameters[i] = node->type->parameters[i]->getRef();
            node->parameters[i].type.push_back('*');
            node->parameters[i].name = getTmpParamName(node->parameters[i].name);

            node->body->addLocalVar(node->parameters[i].name, node->type->parameters[i]);

            auto defNode = std::make_unique<AsgVariableDefinition>();
            defNode->name = origName;
            defNode->type = origTypeStr;
            defNode->list = (AsgStatementList*)node->body.get();
            defNode->function = node;

            auto derefOp = std::make_unique<AsgOpDeref>();
            derefOp->derefCount = 1;
            derefOp->list = (AsgStatementList*)node->body.get();
            derefOp->function = node;

            auto varNode = std::make_unique<AsgVariable>();
            varNode->name = node->parameters[i].name;
            varNode->list = (AsgStatementList*)node->body.get();
            varNode->function = node;

            derefOp->expression = std::move(varNode);
            defNode->value = std::move(derefOp);

            auto* stList = (AsgStatementList*)node->body.get();
            stList->statements.insert(stList->statements.begin(), std::move(defNode));
        }
    }

    if (node->type->returnType->as<StructType>()) {
        node->body->addLocalVar(getTmpRetName(node->name), node->type->returnType->getRef());

        node->parameters.insert(node->parameters.begin(), {node->returnType, getTmpRetName(node->name)});
        node->type->parameters.insert(node->type->parameters.begin(), node->type->returnType->getRef());

        node->type->origRetType = node->type->returnType;

        node->type->returnType = TypeLibrary::inst().get("void");
        node->returnType = "void";
    }

    node->body->accept(this);

    return LRValue{Type::invalid()};
}

std::any TypeResolver::visitVariableDefinition(struct AsgVariableDefinition* node)
{
    UPDATE_VIS(node, nodes_);
    auto nodeType = TypeLibrary::inst().get(node->type);
    if (node->value) {
        auto valueType = std::any_cast<LRValue>(node->value->accept(this)).type;
        if (!Type::isSame(nodeType, valueType)) {
            if (nodeType && valueType) {
                TC_LOG_ERROR(
                    "at line {} -- invalid type conversion in assignment: {} -> {}",
                    node->refLine,
                    valueType->toString(),
                    nodeType->toString());
            }
            ok_ = false;
        }
    }
    return LRValue{Type::invalid()};
}

std::any TypeResolver::visitReturn(struct AsgReturn* node)
{
    UPDATE_VIS(node, nodes_);
    auto expected = node->function->type->returnType;
    if (node->function->type->origRetType) {
        expected = node->function->type->origRetType;
    }
    auto real = std::any_cast<LRValue>(node->value->accept(this)).type;
    if (!Type::isSame(real, expected)) {
        if (real && expected) {
            TC_LOG_ERROR(
                "at line {} -- invalid return type: expected {}, got {}",
                node->refLine,
                expected->toString(),
                real->toString());
        }
        ok_ = false;
    }

    if (node->function->type->origRetType) {
        auto assignNode = std::make_unique<AsgAssignment>();
        assignNode->list = node->list;
        assignNode->function = node->function;
        assignNode->exprType = node->function->type->origRetType;

        auto assignable = std::make_unique<AsgOpDeref>();
        assignable->list = node->list;
        assignable->function = node->function;
        assignable->derefCount = 1;
        assignable->exprType = node->function->type->origRetType;

        auto var = std::make_unique<AsgVariable>();
        var->list = node->list;
        var->function = node->function;
        var->exprType = node->function->type->origRetType->getRef();
        var->name = getTmpRetName(node->function->name);

        assignable->expression = std::move(var);
        assignNode->assignable = std::move(assignable);
        assignNode->value.swap(node->value);

        node->list->statements.insert(
            std::find_if(
                node->list->statements.begin(),
                node->list->statements.end(),
                [&](const std::unique_ptr<AsgNode>& n) { return n.get() == node; }),
            std::move(assignNode));
    }

    return LRValue{Type::invalid()};
}

std::any TypeResolver::visitAssignment(struct AsgAssignment* node)
{
    UPDATE_VIS(node, nodes_);
    auto valueType = std::any_cast<LRValue>(node->value->accept(this));
    auto assignableType = std::any_cast<LRValue>(node->assignable->accept(this));

    if (assignableType.side == LRValue::Side::R) {
        TC_LOG_ERROR(
            "at line {} -- can not assign to rvalue if type {}",
            node->refLine,
            assignableType.type->toString());
        return LRValue{Type::invalid()};
    }

    if (!Type::isSame(valueType.type, assignableType.type)) {
        if (valueType.type && assignableType.type) {
            TC_LOG_ERROR(
                "at line {} -- invalid type conversion in assignment: {} -> {}",
                node->refLine,
                valueType.type->toString(),
                assignableType.type->toString());
        }
        ok_ = false;
        return LRValue{Type::invalid()};
    }
    node->exprType = valueType.type;
    return LRValue{valueType};
}

std::any TypeResolver::visitConditional(struct AsgConditional* node)
{
    UPDATE_VIS(node, nodes_);
    node->condition->accept(this);
    node->thenNode->accept(this);
    if (node->elseNode) {
        node->elseNode->accept(this);
    }
    return LRValue{Type::invalid()};
}

std::any TypeResolver::visitLoop(struct AsgLoop* node)
{
    UPDATE_VIS(node, nodes_);
    node->condition->accept(this);
    node->body->accept(this);
    return LRValue{Type::invalid()};
}

std::any TypeResolver::visitComp(struct AsgComp* node)
{
    UPDATE_VIS(node, nodes_);
    auto lhsType = std::any_cast<LRValue>(node->lhs->accept(this)).type;
    auto rhsType = std::any_cast<LRValue>(node->rhs->accept(this)).type;
    if (!Type::isSame(lhsType, rhsType)) {
        if (rhsType && lhsType) {
            TC_LOG_ERROR(
                "at line {} -- comparison between values of different types: {} vs {}",
                node->refLine,
                lhsType->toString(),
                rhsType->toString());
        }
        ok_ = false;
        return LRValue{Type::invalid()};
    }
    node->exprType = TypeLibrary::inst().get("int");
    return LRValue{node->exprType};
}

std::any TypeResolver::visitAddSub(struct AsgAddSub* node)
{
    UPDATE_VIS(node, nodes_);
    for (auto& expr : node->subexpressions) {
        if (auto t = std::any_cast<LRValue>(expr.expression->accept(this)).type; t != TypeLibrary::inst().get("int")) {
            if (t) {
                TC_LOG_ERROR("at line {} -- invalid arithmetic with type {}", node->refLine, t->toString());
            }
            ok_ = false;
            return LRValue{Type::invalid()};
        }
    }
    node->exprType = TypeLibrary::inst().get("int");
    return LRValue{node->exprType};
}

std::any TypeResolver::visitMulDiv(struct AsgMulDiv* node)
{
    UPDATE_VIS(node, nodes_);
    for (auto& expr : node->subexpressions) {
        if (auto t = std::any_cast<LRValue>(expr.expression->accept(this)).type; t != TypeLibrary::inst().get("int")) {
            if (t) {
                TC_LOG_ERROR("at line {} -- invalid arithmetic with type {}", node->refLine, t->toString());
            }
            ok_ = false;
            return LRValue{Type::invalid()};
        }
    }
    node->exprType = TypeLibrary::inst().get("int");
    return LRValue{node->exprType};
}

std::any TypeResolver::visitFieldAccess(struct AsgFieldAccess* node)
{
    UPDATE_VIS(node, nodes_);

    auto accessedT = std::any_cast<LRValue>(node->accessed->accept(this));

    if (!accessedT.type) {
        return LRValue{Type::invalid()};
    }

    if (!accessedT.type->as<StructType>()) {
        TC_LOG_ERROR(
            "at line {} -- can not access field of var of type {}",
            node->refLine,
            accessedT.type->toString());
        ok_ = false;
        return LRValue{Type::invalid()};
    }

    if (accessedT.type->as<StructType>()->getFieldId(node->field) == -1) {
        TC_LOG_ERROR(
            "at line {} -- type {} has no field {}",
            node->refLine,
            accessedT.type->toString(),
            node->field);
        ok_ = false;
        return LRValue{Type::invalid()};
    }

    node->exprType = accessedT.type->as<StructType>()->getFieldType(node->field);
    return LRValue{node->exprType, accessedT.side};
}

std::any TypeResolver::visitIndexing(struct AsgIndexing* node)
{
    UPDATE_VIS(node, nodes_);
    auto indexedType = std::any_cast<LRValue>(node->indexed->accept(this)).type;
    if (!indexedType || !indexedType->as<ArrayType>()) {
        if (indexedType) {
            TC_LOG_ERROR("at line {} -- invalid indexing of type {}", node->refLine, indexedType->toString());
        }
        ok_ = false;
        return LRValue{Type::invalid()};
    }
    for (auto& index : node->indexes) {
        auto indexType = std::any_cast<LRValue>(index->accept(this)).type;
        if (indexType != TypeLibrary::inst().get("int")) {
            if (indexType) {
                TC_LOG_ERROR(
                    "at line {} -- invalid index type: expected: int, got: {}",
                    index->refLine,
                    indexedType->toString());
            }
            ok_ = false;
            return LRValue{Type::invalid()};
        }
    }

    auto prevType = indexedType;

    for (auto& i : node->indexes) {
        if (!indexedType || !indexedType->as<ArrayType>()) {
            if (prevType) {
                TC_LOG_ERROR("at line {} -- invalid indexing: cant index {}", i->refLine, prevType->toString());
            }
            ok_ = false;
            return LRValue{Type::invalid()};
        }
        prevType = indexedType;
        indexedType = indexedType->as<ArrayType>()->getIndexed();
    }

    node->exprType = indexedType;
    return LRValue{indexedType, LRValue::Side::L};
}

std::any TypeResolver::visitOpDeref(struct AsgOpDeref* node)
{
    UPDATE_VIS(node, nodes_);
    auto t = std::any_cast<LRValue>(node->expression->accept(this)).type;

    if (!t) {
        return LRValue{Type::invalid()};
    }

    auto prevT = t;

    for (auto i = 0; i < node->derefCount; i++) {
        if (!t || !t->as<PtrType>()) {
            if (prevT) {
                TC_LOG_ERROR("at line {} -- cant deref {}", node->refLine, prevT->toString());
            }
            ok_ = false;
            return LRValue{Type::invalid()};
        }
        prevT = t;
        t = t->as<PtrType>()->getDeref();
    }

    node->exprType = t;
    return LRValue{t, LRValue::Side::L};
}

std::any TypeResolver::visitOpRef(struct AsgOpRef* node)
{
    UPDATE_VIS(node, nodes_);
    auto t = std::any_cast<LRValue>(node->value->accept(this));

    if (!t.type) {
        return LRValue(Type::invalid());
    }

    if (t.side == LRValue::Side::R) {
        TC_LOG_ERROR(
            "at line {} -- can not get address of rvalue of type {}",
            node->refLine,
            t.type->toString());
        ok_ = false;
        return LRValue(Type::invalid());
    }

    node->exprType = t.type->getRef();
    return LRValue{node->exprType};
}

std::any TypeResolver::visitVariable(struct AsgVariable* node)
{
    UPDATE_VIS(node, nodes_);
    const AsgStatementList* list = node->list;
    while (list) {
        if (list->localVariables.find(node->name) != list->localVariables.end()) {
            node->exprType = list->localVariables.at(node->name);
            return LRValue(node->exprType, LRValue::Side::L);
        }
        list = list->list;
    }
    return LRValue(Type::invalid());
}

std::any TypeResolver::visitCall(struct AsgCall* node)
{
    UPDATE_VIS(node, nodes_);

    const auto origParamCount = node->callee->origRetType
                                  ? node->callee->parameters.size() - 1
                                  : node->callee->parameters.size();

    if (origParamCount != node->arguments.size()) {
        TC_LOG_ERROR(
            "at line {} -- invalid {} call arg count: expected: {}, got: {}",
            node->refLine,
            node->functionName,
            origParamCount,
            node->arguments.size());
        ok_ = false;
        return LRValue{Type::invalid()};
    }
    for (auto i = 0; i < node->arguments.size(); i++) {

        auto argT = std::any_cast<LRValue>(node->arguments[i]->accept(this)).type;
        auto realParamT = node->callee->parameters[node->callee->origRetType ? i + 1 : i];

        if (!argT) {
            return LRValue{Type::invalid()};
        }

        if (argT->as<StructType>()) {
            realParamT = node->callee->parameters[i]->as<PtrType>()->getDeref();
        }

        if (!Type::isSame(realParamT, argT)) {
            if (argT) {
                TC_LOG_ERROR(
                    "at line {} -- invalid arg #{} type: expected: {}, got: {}",
                    node->arguments[i]->refLine,
                    i + 1,
                    node->callee->parameters[i]->toString(),
                    argT->toString());
            }
            ok_ = false;
            return LRValue{Type::invalid()};
        }

        if (argT->as<StructType>()) {
            auto refNode = std::make_unique<AsgOpRef>();
            refNode->list = node->list;
            refNode->function = node->function;
            refNode->value = std::move(node->arguments[i]);
            node->arguments[i] = std::move(refNode);
            node->arguments[i]->accept(this);
        }
    }

    if (node->callee->origRetType) {
        nodes_.pop_back();

        auto tmpName = "." + std::to_string(next_unique_tmp_++) + "_ret_val";
        auto declNode = std::make_unique<AsgVariableDefinition>();
        declNode->list = node->list;
        declNode->function = node->function;
        declNode->type = node->callee->origRetType->toString();
        declNode->name = tmpName;
        node->list->statements.insert(node->list->statements.begin(), std::move(declNode));

        node->addLocalVar(tmpName, node->callee->origRetType);

        {
            auto varNode = std::make_unique<AsgVariable>();
            varNode->list = node->list;
            varNode->function = node->function;
            varNode->exprType = node->callee->origRetType;
            varNode->name = tmpName;

            nodes_.back()->updateChild(node, varNode.release());
        }

        auto refOp = std::make_unique<AsgOpRef>();
        refOp->list = node->list;
        refOp->function = node->function;
        refOp->exprType = node->callee->origRetType->getRef();

        {
            auto varNode = std::make_unique<AsgVariable>();
            varNode->list = node->list;
            varNode->function = node->function;
            varNode->exprType = node->callee->origRetType;
            varNode->name = tmpName;
            refOp->value = std::move(varNode);
        }

        node->arguments.insert(node->arguments.begin(), std::move(refOp));

        node->list->statements.insert(
            std::find_if(
                node->list->statements.begin(),
                node->list->statements.end(),
                [&](const std::unique_ptr<AsgNode>& n) {
                    for (auto p = nodes_.rbegin(); p != nodes_.rend(); p++) {
                        if (*p == n.get()) {
                            return true;
                        }
                    }
                    return false;
                }),
            std::unique_ptr<AsgNode>(node));

        nodes_.push_back(node);
    }

    node->exprType = node->callee->origRetType ? node->callee->origRetType : node->callee->returnType;
    return LRValue{node->exprType};
}

std::any TypeResolver::visitIntLiteral(struct AsgIntLiteral* node)
{
    UPDATE_VIS(node, nodes_);
    node->exprType = TypeLibrary::inst().get("int");
    return LRValue{node->exprType};
}
