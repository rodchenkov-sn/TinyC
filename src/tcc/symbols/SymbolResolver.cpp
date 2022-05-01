#include "SymbolResolver.h"

#include "asg/AsgNode.h"

#include "TypeLib.h"
#include "FunctionLib.h"


bool SymbolResolver::resolve(AsgNode* root)
{
    root->accept(this);
    return errors_.empty();
}


std::any SymbolResolver::visitStatementList(struct AsgStatementList* node)
{
    node->parent = top_scope_;
    top_scope_ = node;

    node->function = current_function_;

    for (auto& n : node->statements) {
        n->accept(this);
    }

    top_scope_ = node->parent;

    return {};
}


std::any SymbolResolver::visitFunctionDefinition(struct AsgFunctionDefinition* node)
{
    Function function;
    function.name = node->name;

    auto* retType = TypeLibrary::inst().get(node->returnType);

    if (!retType) {
        errors_.emplace_back() << "undefined return type " << node->returnType << " in " << function.name << " signature";
    }

    function.returnType = retType;

    auto* topBlock = (AsgStatementList*) node->body.get();

    for (auto& parameter : node->parameters) {
        auto* type = TypeLibrary::inst().get(parameter.type);
        if (!type) {
            errors_.emplace_back() << "undefined parameter type " << parameter.type << " in " << function.name << " signature";
        }
        function.parameters.push_back(type);

        topBlock->localVariables.insert({ parameter.name, type });
    }

    node->type = FunctionLibrary::inst().add(function);

    if (!node->type) {
        errors_.emplace_back() << "function " << node->name << " redefinition";
    }

    current_function_ = node;

    node->body->accept(this);

    current_function_ = nullptr;

    return {};
}


std::any SymbolResolver::visitVariableDefinition(struct AsgVariableDefinition* node)
{
    node->function = current_function_;
    node->parent = top_scope_;

    if (findVarType(node->name)) {
        errors_.emplace_back() << "variable " << node->name << " was redefined";
    }

    auto* type = TypeLibrary::inst().get(node->type);
    if (!type) {
        errors_.emplace_back() << "undefined type " << node->type << " in variable " << node->name << " definition";
    }

    top_scope_->localVariables.insert({ node->name, type });

    if (node->value) {
        node->value->accept(this);
    }

    return {};
}


std::any SymbolResolver::visitReturn(struct AsgReturn* node)
{
    node->function = current_function_;
    node->parent = top_scope_;

    node->value->accept(this);

    return {};
}


std::any SymbolResolver::visitAssignment(struct AsgAssignment* node)
{
    node->function = current_function_;
    node->parent = top_scope_;

    if (!findVarType(node->name)) {
        errors_.emplace_back() << "variable " << node->name << " is undefined";
    }

    node->value->accept(this);

    return {};
}


std::any SymbolResolver::visitComp(struct AsgComp* node)
{
    node->function = current_function_;
    node->parent = top_scope_;

    node->rhs->accept(this);
    node->lhs->accept(this);

    return {};
}


std::any SymbolResolver::visitAddSub(struct AsgAddSub* node)
{
    node->function = current_function_;
    node->parent = top_scope_;

    for (auto& subexpression : node->subexpressions) {
        subexpression.expression->accept(this);
    }

    return {};
}


std::any SymbolResolver::visitMulDiv(struct AsgMulDiv* node)
{
    node->function = current_function_;
    node->parent = top_scope_;

    for (auto& subexpression : node->subexpressions) {
        subexpression.expression->accept(this);
    }

    return {};
}


std::any SymbolResolver::visitVariable(struct AsgVariable* node)
{
    node->function = current_function_;
    node->parent = top_scope_;

    if (!findVarType(node->name)) {
        errors_.emplace_back() << "variable " << node->name << " is undefined";
    }

    return {};
}


std::any SymbolResolver::visitCall(struct AsgCall* node)
{
    node->function = current_function_;
    node->parent = top_scope_;

    if (!FunctionLibrary::inst().get(node->functionName)) {
        errors_.emplace_back() << "function " << node->functionName << " is undefined";
    }

    for (auto& arg : node->arguments) {
        arg->accept(this);
    }

    return {};
}


std::any SymbolResolver::visitIntLiteral(struct AsgIntLiteral* node)
{
    node->function = current_function_;
    node->parent = top_scope_;

    return {};
}


TypeId SymbolResolver::findVarType(const std::string& name) const
{
    auto* currScope = top_scope_;
    while (currScope) {
        if (currScope->localVariables.find(name) != currScope->localVariables.end()) {
            return currScope->localVariables.at(name);
        }
        currScope = currScope->parent;
    }
    return nullptr;
}
