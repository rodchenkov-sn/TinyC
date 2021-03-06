#ifndef TINYC_SYMBOLRESOLVER_H
#define TINYC_SYMBOLRESOLVER_H

#include "asg/AsgNode.h"
#include "asg/AsgVisitor.h"
#include "pipeline/PipelineStage.h"
#include "Type.h"

class SymbolResolver : private AsgVisitorBase,
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
    std::any visitFieldAccess(struct AsgFieldAccess* node) override;
    std::any visitIndexing(struct AsgIndexing* node) override;
    std::any visitOpDeref(struct AsgOpDeref* node) override;
    std::any visitOpRef(struct AsgOpRef* node) override;
    std::any visitVariable(struct AsgVariable* node) override;
    std::any visitCall(struct AsgCall* node) override;
    std::any visitIntLiteral(struct AsgIntLiteral* node) override;

    Type::Id findVarType(const std::string& name) const;

    AsgStatementList* top_scope_ = nullptr;
    AsgFunctionDefinition* current_function_ = nullptr;

    bool ok_ = true;
};

#endif
