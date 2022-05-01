#include "SymbolResolver.h"

#include "asg/AsgNode.h"

#include "TypeLib.h"
#include "FunctionLib.h"


bool SymbolResolver::resolve(AsgNode* root)
{
    root->accept(this);
    return errors_.empty();
}


void SymbolResolver::visitStatementList(struct AsgStatementList* node)
{
    for (auto& n : node->statements) {
        n->accept(this);
    }
}


void SymbolResolver::visitFunctionDefinition(struct AsgFunctionDefinition* node)
{
    local_variables_.emplace();

    Function function;
    function.name = node->name;

    auto* retType = TypeLibrary::inst().get(node->returnType);

    if (!retType) {
        errors_.emplace_back() << "undefined return type " << node->returnType << " in " << function.name << " signature";
    }

    function.returnType = retType;

    for (auto& parameter : node->parameters) {
        auto* type = TypeLibrary::inst().get(parameter.type);
        if (!type) {
            errors_.emplace_back() << "undefined parameter type " << parameter.type << " in " << function.name << " signature";
        }
        function.parameters.push_back(type);

        local_variables_.top().insert({ parameter.name, type });
    }

    FunctionLibrary::inst().add(function);

    node->body->accept(this);
    local_variables_.pop();
}


void SymbolResolver::visitVariableDefinition(struct AsgVariableDefinition* node)
{
    if (local_variables_.top().find(node->name) != local_variables_.top().end()) {
        errors_.emplace_back() << "variable " << node->name << " was redefined";
    }

    auto* type = TypeLibrary::inst().get(node->type);
    if (!type) {
        errors_.emplace_back() << "undefined type " << node->type << " in variable " << node->name << " definition";
    }

    local_variables_.top().insert({ node->name, type });

    if (node->value) {
        node->value->accept(this);
    }
}


void SymbolResolver::visitReturn(struct AsgReturn* node)
{
    node->value->accept(this);
}


void SymbolResolver::visitAssignment(struct AsgAssignment* node)
{
    if (local_variables_.top().find(node->name) == local_variables_.top().end()) {
        errors_.emplace_back() << "variable " << node->name << " is undefined";
    }

    node->value->accept(this);
}


void SymbolResolver::visitAddSub(struct AsgAddSub* node)
{
    for (auto& subexpression : node->subexpressions) {
        subexpression.expression->accept(this);
    }
}


void SymbolResolver::visitMulDiv(struct AsgMulDiv* node)
{
    for (auto& subexpression : node->subexpressions) {
        subexpression.expression->accept(this);
    }
}


void SymbolResolver::visitVariable(struct AsgVariable* node)
{
    if (local_variables_.top().find(node->name) == local_variables_.top().end()) {
        errors_.emplace_back() << "variable " << node->name << " is undefined";
    }
}


void SymbolResolver::visitCall(struct AsgCall* node)
{
    if (!FunctionLibrary::inst().get(node->functionName)) {
        errors_.emplace_back() << "function " << node->functionName << " is undefined";
    }

    for (auto& arg : node->arguments) {
        arg->accept(this);
    }
}


void SymbolResolver::visitIntLiteral(struct AsgIntLiteral* node)
{
}
