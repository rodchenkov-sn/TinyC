#ifndef TINYC_ASGVISITOR_H
#define TINYC_ASGVISITOR_H


#include <any>


struct AsgVisitorBase {
    virtual std::any visitStatementList(struct AsgStatementList* node) = 0;
    virtual std::any visitFunctionDefinition(struct AsgFunctionDefinition* node) = 0;
    virtual std::any visitVariableDefinition(struct AsgVariableDefinition* node) = 0;
    virtual std::any visitReturn(struct AsgReturn* node) = 0;
    virtual std::any visitAssignment(struct AsgAssignment* node) = 0;
    virtual std::any visitConditional(struct AsgConditional* node) = 0;
    virtual std::any visitLoop(struct AsgLoop* node) = 0;

    virtual std::any visitComp(struct AsgComp* node) = 0;
    virtual std::any visitAddSub(struct AsgAddSub* node) = 0;
    virtual std::any visitMulDiv(struct AsgMulDiv* node) = 0;
    virtual std::any visitOpDeref(struct AsgOpDeref* node) = 0;
    virtual std::any visitOpRef(struct AsgOpRef* node) = 0;
    virtual std::any visitVariable(struct AsgVariable* node) = 0;
    virtual std::any visitCall(struct AsgCall* node) = 0;
    virtual std::any visitIntLiteral(struct AsgIntLiteral* node) = 0;
};


#endif // TINYC_ASGVISITOR_H
