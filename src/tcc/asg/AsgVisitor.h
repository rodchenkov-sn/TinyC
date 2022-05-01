#ifndef TINYC_ASGVISITOR_H
#define TINYC_ASGVISITOR_H


struct AsgVisitorBase {
    virtual void visitStatementList(struct AsgStatementList* node) = 0;
    virtual void visitFunctionDefinition(struct AsgFunctionDefinition* node) = 0;
    virtual void visitVariableDefinition(struct AsgVariableDefinition* node) = 0;
    virtual void visitReturn(struct AsgReturn* node) = 0;

    virtual void visitAssignment(struct AsgAssignment* node) = 0;
    virtual void visitAddSub(struct AsgAddSub* node) = 0;
    virtual void visitMulDiv(struct AsgMulDiv* node) = 0;
    virtual void visitVariable(struct AsgVariable* node) = 0;
    virtual void visitCall(struct AsgCall* node) = 0;
    virtual void visitIntLiteral(struct AsgIntLiteral* node) = 0;
};


#endif // TINYC_ASGVISITOR_H
