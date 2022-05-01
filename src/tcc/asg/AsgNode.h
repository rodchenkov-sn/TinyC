#ifndef TINYC_ASGNODE_H
#define TINYC_ASGNODE_H

#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <string>

#include "AsgVisitor.h"


struct AsgNode {
    virtual ~AsgNode() = default;

    virtual void accept(AsgVisitorBase *visitor) = 0;
    virtual void print(std::string indent) const = 0;
};


struct AsgStatementList : AsgNode {
    void accept(AsgVisitorBase *visitor) override { visitor->visitStatementList(this); }

    void print(std::string indent) const override
    {
        for (auto& s : statements) {
            s->print(indent);
        }
    }

    std::vector<std::unique_ptr<AsgNode>> statements;
};


struct AsgFunctionDefinition : AsgNode {
    struct Parameter {
        std::string type;
        std::string name;
    };

    void accept(AsgVisitorBase* visitor) override { visitor->visitFunctionDefinition(this); }

    void print(std::string indent) const override
    {
        std::cout << "fun " << name;
        for (auto& p : parameters) {
            std::cout << " " << p.name << ':' << p.type;
        }
        std::cout << " -> " << returnType << "\n";
        if (body) {
            body->print(indent + "  ");
        }
        std::cout << "\n";
    }

    std::string name;
    std::string returnType;
    std::vector<Parameter> parameters;
    std::unique_ptr<AsgNode> body;
};


struct AsgVariableDefinition : AsgNode {
    void accept(AsgVisitorBase* visitor) override { visitor->visitVariableDefinition(this); }

    void print(std::string indent) const override
    {
        std::cout << indent << "def " << name << ':' << type;
        if (value) {
            std::cout << " =\n";
            value->print(indent + "  ");
        }
    }

    std::string type;
    std::string name;
    std::unique_ptr<AsgNode> value;
};

struct AsgReturn : AsgNode {
    void accept(AsgVisitorBase* visitor) override { visitor->visitReturn(this); }

    void print(std::string indent) const override
    {
        std::cout << indent << "ret\n";
        value->print(indent + "  ");
    }

    std::unique_ptr<AsgNode> value;
};


struct AsgAssignment : AsgNode {
    void accept(AsgVisitorBase* visitor) override { visitor->visitAssignment(this); }

    void print(std::string indent) const override
    {
        std::cout << indent << name << " =\n";
        value->print(indent + "  ");
    }

    std::string name;
    std::unique_ptr<AsgNode> value;
};


struct AsgAddSub : AsgNode {
    enum class Operator {
        Add, Sub
    };

    struct Subexpression {
        Operator leadingOp;
        std::unique_ptr<AsgNode> expression;
    };

    void accept(AsgVisitorBase* visitor) override { visitor->visitAddSub(this); }

    void print(std::string indent) const override
    {
        for (auto& s : subexpressions) {
            std::cout << indent << (s.leadingOp == Operator::Add ? '+' : '-') << '\n';
            s.expression->print(indent + "  ");
        }
    }

    std::vector<Subexpression> subexpressions;
};


struct AsgMulDiv : AsgNode {
    enum class Operator {
        Mul, Div
    };

    struct Subexpression {
        Operator leadingOp;
        std::unique_ptr<AsgNode> expression;
    };

    void accept(AsgVisitorBase* visitor) override { visitor->visitMulDiv(this); }

    void print(std::string indent) const override
    {
        for (auto& s : subexpressions) {
            std::cout << indent << (s.leadingOp == Operator::Mul ? '*' : '/') << '\n';
            s.expression->print(indent + "  ");
        }
    }

    std::vector<Subexpression> subexpressions;
};

struct AsgVariable : AsgNode {
    void accept(AsgVisitorBase* visitor) override { visitor->visitVariable(this); }

    void print(std::string indent) const override
    {
        std::cout << indent << "var " << name << '\n';
    }

    std::string name;
};

struct AsgCall : AsgNode {
    void accept(AsgVisitorBase* visitor) override { visitor->visitCall(this); }

    void print(std::string indent) const override
    {
        std::cout << indent << "call " << functionName << "\n";
        for (auto& a : arguments) {
            a->print(indent + "  ");
        }
    }

    std::string functionName;
    std::vector<std::unique_ptr<AsgNode>> arguments;
};

struct AsgIntLiteral : AsgNode {
    void accept(AsgVisitorBase* visitor) override { visitor->visitIntLiteral(this); }

    void print(std::string indent) const override
    {
        std::cout << indent << "literal " << value << '\n';
    }

    int value;
};


#endif // TINYC_ASGNODE_H
