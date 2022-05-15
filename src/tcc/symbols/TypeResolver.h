#ifndef TINYC_TYPERESOLVER_H
#define TINYC_TYPERESOLVER_H

#include <deque>

#include "asg/AsgNode.h"
#include "asg/AsgVisitor.h"
#include "pipeline/PipelineStage.h"

class TypeResolver : private AsgVisitorBase,
                     public PipeModifierBase {
public:
    std::any modify(std::any data) override;

private:
    std::any visitStatementList(struct AsgStatementList* node) override;
    std::any visitStructDefinition(struct AsgStructDefinition* node) override;
    std::any visitFunctionDefinition(struct AsgFunctionDefinition* node) override;
    std::any visitVariableDefinition(struct AsgVariableDefinition* node) override;
    std::any visitReturn(struct AsgReturn* node) override;
    std::any visitAssignment(struct AsgAssignment* node) override;
    std::any visitConditional(struct AsgConditional* node) override;
    std::any visitLoop(struct AsgLoop* node) override;
    std::any visitComp(struct AsgComp* node) override;
    std::any visitAddSub(struct AsgAddSub* node) override;
    std::any visitMulDiv(struct AsgMulDiv* node) override;
    std::any visitIndexing(struct AsgIndexing* node) override;
    std::any visitOpDeref(struct AsgOpDeref* node) override;
    std::any visitOpRef(struct AsgOpRef* node) override;
    std::any visitVariable(struct AsgVariable* node) override;
    std::any visitCall(struct AsgCall* node) override;
    std::any visitIntLiteral(struct AsgIntLiteral* node) override;

    bool ok_ = true;
    int next_unique_tmp_ = 0;

    std::deque<AsgNode*> nodes_;
};

#endif
