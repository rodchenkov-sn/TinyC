#ifndef TINYC_ASGNODE_H
#define TINYC_ASGNODE_H

#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "AsgVisitor.h"
#include "symbols/Function.h"
#include "symbols/Type.h"

struct AsgNode {
    virtual ~AsgNode() = default;

    virtual std::any accept(AsgVisitorBase* visitor) = 0;

    AsgNode* parent = nullptr;

    struct AsgStatementList* list = nullptr;
    struct AsgFunctionDefinition* function = nullptr;
};

struct AsgStatementList : AsgNode {
    std::any accept(AsgVisitorBase* visitor) override
    {
        return visitor->visitStatementList(this);
    }

    std::vector<std::unique_ptr<AsgNode>> statements;
    std::unordered_map<std::string, Type::Id> localVariables;
};

struct AsgFunctionDefinition : AsgNode {
    struct Parameter {
        std::string type;
        std::string name;
    };

    std::any accept(AsgVisitorBase* visitor) override
    {
        return visitor->visitFunctionDefinition(this);
    }

    std::string name;
    std::string returnType;
    std::vector<Parameter> parameters;
    std::unique_ptr<AsgNode> body;

    FunctionId type = nullptr;
};

struct AsgVariableDefinition : AsgNode {
    std::any accept(AsgVisitorBase* visitor) override
    {
        return visitor->visitVariableDefinition(this);
    }

    std::string type;
    std::string name;
    std::unique_ptr<AsgNode> value;
};

struct AsgReturn : AsgNode {
    std::any accept(AsgVisitorBase* visitor) override
    {
        return visitor->visitReturn(this);
    }

    std::unique_ptr<AsgNode> value;
};

struct AsgAssignment : AsgNode {
    std::any accept(AsgVisitorBase* visitor) override
    {
        return visitor->visitAssignment(this);
    }

    std::unique_ptr<AsgNode> assignable;
    std::unique_ptr<AsgNode> value;
};

struct AsgConditional : AsgNode {
    std::any accept(AsgVisitorBase* visitor) override
    {
        return visitor->visitConditional(this);
    }

    std::unique_ptr<AsgNode> condition;
    std::unique_ptr<AsgNode> thenNode;
    std::unique_ptr<AsgNode> elseNode;
};

struct AsgLoop : AsgNode {
    std::any accept(AsgVisitorBase* visitor) override
    {
        return visitor->visitLoop(this);
    }

    std::unique_ptr<AsgNode> condition;
    std::unique_ptr<AsgNode> body;
};

struct AsgComp : AsgNode {
    enum class Operator {
        Equals,
        NotEquals,
        Less,
        LessEquals,
        Greater,
        GreaterEquals
    };

    std::any accept(AsgVisitorBase* visitor) override
    {
        return visitor->visitComp(this);
    }

    AsgNode* lhs;
    AsgNode* rhs;
    Operator op;
};

struct AsgAddSub : AsgNode {
    enum class Operator {
        Add,
        Sub
    };

    struct Subexpression {
        Operator leadingOp;
        std::unique_ptr<AsgNode> expression;
    };

    std::any accept(AsgVisitorBase* visitor) override
    {
        return visitor->visitAddSub(this);
    }

    std::vector<Subexpression> subexpressions;
};

struct AsgMulDiv : AsgNode {
    enum class Operator {
        Mul,
        Div
    };

    struct Subexpression {
        Operator leadingOp;
        std::unique_ptr<AsgNode> expression;
    };

    std::any accept(AsgVisitorBase* visitor) override
    {
        return visitor->visitMulDiv(this);
    }

    std::vector<Subexpression> subexpressions;
};

struct AsgIndexing : AsgNode {
    std::any accept(AsgVisitorBase* visitor) override
    {
        return visitor->visitIndexing(this);
    }

    std::unique_ptr<AsgNode> indexed;
    std::vector<std::unique_ptr<AsgNode>> indexes;
};

struct AsgOpDeref : AsgNode {
    std::any accept(AsgVisitorBase* visitor) override
    {
        return visitor->visitOpDeref(this);
    }

    size_t derefCount;
    std::unique_ptr<AsgNode> expression;
};

struct AsgOpRef : AsgNode {
    std::any accept(AsgVisitorBase* visitor) override
    {
        return visitor->visitOpRef(this);
    }

    std::unique_ptr<AsgNode> value;
};

struct AsgVariable : AsgNode {
    std::any accept(AsgVisitorBase* visitor) override
    {
        return visitor->visitVariable(this);
    }

    std::string name;
};

struct AsgCall : AsgNode {
    std::any accept(AsgVisitorBase* visitor) override
    {
        return visitor->visitCall(this);
    }

    std::string functionName;
    FunctionId callee;
    std::vector<std::unique_ptr<AsgNode>> arguments;
};

struct AsgIntLiteral : AsgNode {
    std::any accept(AsgVisitorBase* visitor) override
    {
        return visitor->visitIntLiteral(this);
    }

    int value;
};

#endif
