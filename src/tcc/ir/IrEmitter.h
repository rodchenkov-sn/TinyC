#ifndef TINYC_IREMITTER_H
#define TINYC_IREMITTER_H

#include "asg/AsgNode.h"
#include "asg/AsgVisitor.h"
#include "pipeline/PipelineStage.h"

class IrEmitter : private AsgVisitorBase,
                  public PipeModifierBase {
public:
    IrEmitter(std::string moduleName, bool optimize);

    std::any modify(std::any data) override;

private:
    enum class RetType {
        Ptr,
        Data,
        CallParam,
        Undef
    };

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

    llvm::AllocaInst* findAlloca(const std::string& name) const;
    llvm::AllocaInst* makeAlloca(const std::string& name, llvm::Type* type);

    std::unique_ptr<llvm::LLVMContext> context_;
    std::unique_ptr<llvm::Module> module_;
    std::unique_ptr<llvm::IRBuilder<>> builder_;

    std::deque<std::unordered_map<std::string, llvm::AllocaInst*>> scopes_;
    llvm::Function* curr_function_ = nullptr;

    std::stack<RetType> expected_ret_;

    std::string module_name_;
    bool optimize_;

    bool ok_ = true;
};

#endif
