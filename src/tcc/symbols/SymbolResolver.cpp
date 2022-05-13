#include "SymbolResolver.h"

#include "asg/AsgNode.h"
#include "FunctionLib.h"
#include "TypeLib.h"

bool SymbolResolver::resolve(AsgNode* root)
{
    root->accept(this);
    return errors_.empty();
}

std::any SymbolResolver::visitStatementList(struct AsgStatementList* node)
{
    node->list = top_scope_;
    top_scope_ = node;

    node->function = current_function_;

    for (auto& n : node->statements) {
        n->accept(this);
        n->parent = node;
    }

    top_scope_ = node->list;

    return {};
}

std::any SymbolResolver::visitStructDefinition(struct AsgStructDefinition* node)
{
    std::vector<std::pair<Type::Id, std::string>> fields;
    for (auto& field : node->fields) {
        auto type = TypeLibrary::inst().get(field.type);
        if (!type) {
            std::cerr << "undefined type " << field.type << " in struct " << node->name << '\n';
            continue;
        }
        fields.emplace_back(type, field.name);
    }
    if (!TypeLibrary::inst().add(node->name, std::make_shared<StructType>(fields))) {
        std::cerr << "redefinition of struct " << node->name << '\n';
    }
    return {};
}

std::any SymbolResolver::visitFunctionDefinition(struct AsgFunctionDefinition* node)
{
    Function function;
    function.name = node->name;

    auto retType = TypeLibrary::inst().get(node->returnType);

    if (!retType) {
        errors_.emplace_back() << "undefined return type " << node->returnType << " in " << function.name << " signature";
    }

    function.returnType = retType;

    auto* topBlock = (AsgStatementList*)node->body.get();

    for (auto& parameter : node->parameters) {
        auto type = TypeLibrary::inst().get(parameter.type);
        if (!type) {
            errors_.emplace_back() << "undefined parameter type " << parameter.type << " in " << function.name << " signature";
        }
        function.parameters.push_back(type);

        topBlock->localVariables.insert({parameter.name, type});
    }

    node->type = FunctionLibrary::inst().add(function);

    if (!node->type) {
        errors_.emplace_back() << "function " << node->name << " redefinition";
    }

    current_function_ = node;

    node->body->accept(this);
    node->body->parent = node;

    current_function_ = nullptr;

    return {};
}

std::any SymbolResolver::visitVariableDefinition(struct AsgVariableDefinition* node)
{
    node->function = current_function_;
    node->list = top_scope_;

    if (findVarType(node->name)) {
        errors_.emplace_back() << "variable " << node->name << " was redefined";
    }

    auto type = TypeLibrary::inst().get(node->type);
    if (!type) {
        errors_.emplace_back() << "undefined type " << node->type << " in variable " << node->name << " definition";
    }

    top_scope_->localVariables.insert({node->name, type});

    if (node->value) {
        node->value->accept(this);
        node->value->parent = node;
    }

    return {};
}

std::any SymbolResolver::visitReturn(struct AsgReturn* node)
{
    node->function = current_function_;
    node->list = top_scope_;

    node->value->accept(this);
    node->value->parent = node;

    return {};
}

std::any SymbolResolver::visitAssignment(struct AsgAssignment* node)
{
    node->function = current_function_;
    node->list = top_scope_;

    node->assignable->accept(this);
    node->value->accept(this);
    node->value->parent = node;

    return {};
}

std::any SymbolResolver::visitConditional(struct AsgConditional* node)
{
    node->function = current_function_;
    node->list = top_scope_;

    node->condition->accept(this);
    node->condition->parent = node;

    node->thenNode->accept(this);
    node->thenNode->parent = node;

    if (node->elseNode) {
        node->elseNode->accept(this);
        node->elseNode->parent = node;
    }

    return {};
}

std::any SymbolResolver::visitLoop(struct AsgLoop* node)
{
    node->function = current_function_;
    node->list = top_scope_;

    node->condition->accept(this);
    node->body->accept(this);
    return {};
}

std::any SymbolResolver::visitComp(struct AsgComp* node)
{
    node->function = current_function_;
    node->list = top_scope_;

    node->rhs->accept(this);
    node->rhs->parent = node;

    node->lhs->accept(this);
    node->lhs->parent = node;

    return {};
}

std::any SymbolResolver::visitAddSub(struct AsgAddSub* node)
{
    node->function = current_function_;
    node->list = top_scope_;

    for (auto& subexpression : node->subexpressions) {
        subexpression.expression->accept(this);
        subexpression.expression->parent = node;
    }

    return {};
}

std::any SymbolResolver::visitMulDiv(struct AsgMulDiv* node)
{
    node->function = current_function_;
    node->list = top_scope_;

    for (auto& subexpression : node->subexpressions) {
        subexpression.expression->accept(this);
        subexpression.expression->parent = node;
    }

    return {};
}

std::any SymbolResolver::visitIndexing(struct AsgIndexing* node)
{
    node->function = current_function_;
    node->list = top_scope_;

    node->indexed->accept(this);

    for (auto& index : node->indexes) {
        index->accept(this);
    }

    return {};
}

std::any SymbolResolver::visitOpDeref(struct AsgOpDeref* node)
{
    node->function = current_function_;
    node->list = top_scope_;

    return node->expression->accept(this);
}

std::any SymbolResolver::visitOpRef(struct AsgOpRef* node)
{
    node->function = current_function_;
    node->list = top_scope_;

    return node->value->accept(this);
}

std::any SymbolResolver::visitVariable(struct AsgVariable* node)
{
    node->function = current_function_;
    node->list = top_scope_;

    if (!findVarType(node->name)) {
        errors_.emplace_back() << "variable " << node->name << " is undefined";
    }

    return {};
}

std::any SymbolResolver::visitCall(struct AsgCall* node)
{
    node->function = current_function_;
    node->list = top_scope_;

    auto funId = FunctionLibrary::inst().get(node->functionName);

    if (!funId) {
        errors_.emplace_back() << "function " << node->functionName << " is undefined";
    }

    node->callee = funId;

    for (auto& arg : node->arguments) {
        arg->accept(this);
        arg->parent = node;
    }

    return {};
}

std::any SymbolResolver::visitIntLiteral(struct AsgIntLiteral* node)
{
    node->function = current_function_;
    node->list = top_scope_;

    return {};
}

Type::Id SymbolResolver::findVarType(const std::string& name) const
{
    auto* currScope = top_scope_;
    while (currScope) {
        if (currScope->localVariables.find(name) != currScope->localVariables.end()) {
            return currScope->localVariables.at(name);
        }
        currScope = currScope->list;
    }
    return nullptr;
}
