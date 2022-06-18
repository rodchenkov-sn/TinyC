#include "AsgNode.h"

void AsgNode::addLocalVar(const std::string& name, const Type::Id& type)
{
    if (list) {
        list->addLocalVar(name, type);
    }
}

std::any AsgStatementList::accept(AsgVisitorBase* visitor)
{
    return visitor->visitStatementList(this);
}

void AsgStatementList::updateChild(AsgNode* from, AsgNode* to)
{
    for (auto& s : statements) {
        if (s.get() == from) {
            TC_UNUSED(s.release());
            s.reset(to);
        }
    }
}

void AsgStatementList::addLocalVar(const std::string& name, const Type::Id& type)
{
    localVariables.insert({name, type});
}

std::any AsgStructDefinition::accept(AsgVisitorBase* visitor)
{
    return visitor->visitStructDefinition(this);
}

void AsgStructDefinition::updateChild(AsgNode* from, AsgNode* to)
{
}

std::any AsgFunctionDefinition::accept(AsgVisitorBase* visitor)
{
    return visitor->visitFunctionDefinition(this);
}

void AsgFunctionDefinition::updateChild(AsgNode* from, AsgNode* to)
{
    if (body.get() == from) {
        TC_UNUSED(body.release());
        body.reset(to);
    }
}

std::any AsgVariableDefinition::accept(AsgVisitorBase* visitor)
{
    return visitor->visitVariableDefinition(this);
}

void AsgVariableDefinition::updateChild(AsgNode* from, AsgNode* to)
{
    if (value.get() == from) {
        TC_UNUSED(value.release());
        value.reset(to);
    }
}

std::any AsgReturn::accept(AsgVisitorBase* visitor)
{
    return visitor->visitReturn(this);
}

void AsgReturn::updateChild(AsgNode* from, AsgNode* to)
{
    if (value.get() == from) {
        TC_UNUSED(value.release());
        value.reset(to);
    }
}

std::any AsgAssignment::accept(AsgVisitorBase* visitor)
{
    return visitor->visitAssignment(this);
}

void AsgAssignment::updateChild(AsgNode* from, AsgNode* to)
{
    if (assignable.get() == from) {
        TC_UNUSED(assignable.release());
        assignable.reset(to);
    } else if (value.get() == from) {
        TC_UNUSED(value.release());
        value.reset(to);
    }
}

std::any AsgConditional::accept(AsgVisitorBase* visitor)
{
    return visitor->visitConditional(this);
}

void AsgConditional::updateChild(AsgNode* from, AsgNode* to)
{
    if (condition.get() == from) {
        TC_UNUSED(condition.release());
        condition.reset(to);
    } else if (thenNode.get() == from) {
        TC_UNUSED(thenNode.release());
        thenNode.reset(to);
    } else if (elseNode.get() == from) {
        TC_UNUSED(elseNode.release());
        elseNode.reset(to);
    }
}

std::any AsgLoop::accept(AsgVisitorBase* visitor)
{
    return visitor->visitLoop(this);
}

void AsgLoop::updateChild(AsgNode* from, AsgNode* to)
{
    if (condition.get() == from) {
        TC_UNUSED(condition.release());
        condition.reset(to);
    } else if (body.get() == from) {
        TC_UNUSED(body.release());
        body.reset(to);
    }
}

std::any AsgComp::accept(AsgVisitorBase* visitor)
{
    return visitor->visitComp(this);
}

void AsgComp::updateChild(AsgNode* from, AsgNode* to)
{
    if (lhs.get() == from) {
        TC_UNUSED(lhs.release());
        lhs.reset(to);
    } else if (rhs.get() == from) {
        TC_UNUSED(rhs.release());
        rhs.reset(to);
    }
}

std::any AsgAddSub::accept(AsgVisitorBase* visitor)
{
    return visitor->visitAddSub(this);
}

void AsgAddSub::updateChild(AsgNode* from, AsgNode* to)
{
    for (auto& s : subexpressions) {
        if (s.expression.get() == from) {
            TC_UNUSED(s.expression.release());
            s.expression.reset(to);
        }
    }
}

std::any AsgMulDiv::accept(AsgVisitorBase* visitor)
{
    return visitor->visitMulDiv(this);
}

void AsgMulDiv::updateChild(AsgNode* from, AsgNode* to)
{
    for (auto& s : subexpressions) {
        if (s.expression.get() == from) {
            TC_UNUSED(s.expression.release());
            s.expression.reset(to);
        }
    }
}

std::any AsgFieldAccess::accept(AsgVisitorBase* visitor)
{
    return visitor->visitFieldAccess(this);
}

void AsgFieldAccess::updateChild(AsgNode* from, AsgNode* to)
{
    if (accessed.get() == from) {
        TC_UNUSED(accessed.release());
        accessed.reset(to);
    }
}

std::any AsgIndexing::accept(AsgVisitorBase* visitor)
{
    return visitor->visitIndexing(this);
}

void AsgIndexing::updateChild(AsgNode* from, AsgNode* to)
{
    if (indexed.get() == from) {
        TC_UNUSED(indexed.release());
        indexed.reset(to);
    } else {
        for (auto& i : indexes) {
            if (i.get() == from) {
                TC_UNUSED(i.release());
                i.reset(to);
            }
        }
    }
}

std::any AsgOpDeref::accept(AsgVisitorBase* visitor)
{
    return visitor->visitOpDeref(this);
}

void AsgOpDeref::updateChild(AsgNode* from, AsgNode* to)
{
    if (expression.get() == from) {
        TC_UNUSED(expression.release());
        expression.reset(to);
    }
}

std::any AsgOpRef::accept(AsgVisitorBase* visitor)
{
    return visitor->visitOpRef(this);
}

void AsgOpRef::updateChild(AsgNode* from, AsgNode* to)
{
    if (value.get() == from) {
        TC_UNUSED(value.release());
        value.reset(to);
    }
}

std::any AsgVariable::accept(AsgVisitorBase* visitor)
{
    return visitor->visitVariable(this);
}

void AsgVariable::updateChild(AsgNode* from, AsgNode* to)
{
}

std::any AsgCall::accept(AsgVisitorBase* visitor)
{
    return visitor->visitCall(this);
}

void AsgCall::updateChild(AsgNode* from, AsgNode* to)
{
    for (auto& a : arguments) {
        if (a.get() == from) {
            TC_UNUSED(a.release());
            a.reset(to);
        }
    }
}

std::any AsgIntLiteral::accept(AsgVisitorBase* visitor)
{
    return visitor->visitIntLiteral(this);
}

void AsgIntLiteral::updateChild(AsgNode* from, AsgNode* to)
{
}
