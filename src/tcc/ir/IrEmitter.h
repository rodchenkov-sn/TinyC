#ifndef TINYC_IREMITTER_H
#define TINYC_IREMITTER_H


#include <vector>
#include <sstream>
#include <deque>
#include <unordered_map>

#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LegacyPassManager.h>

#include "asg/AsgVisitor.h"
#include "asg/AsgNode.h"


class IrEmitter : private AsgVisitorBase {
public:
    std::unique_ptr<llvm::Module> emmit(AsgNode* root, std::string_view moduleName, bool optimize = true);

private:
    std::any visitStatementList(struct AsgStatementList* node) override;
    std::any visitFunctionDefinition(struct AsgFunctionDefinition* node) override;
    std::any visitVariableDefinition(struct AsgVariableDefinition* node) override;
    std::any visitReturn(struct AsgReturn* node) override;
    std::any visitAssignment(struct AsgAssignment* node) override;

    std::any visitComp(struct AsgComp *node) override;
    std::any visitAddSub(struct AsgAddSub* node) override;
    std::any visitMulDiv(struct AsgMulDiv* node) override;
    std::any visitVariable(struct AsgVariable* node) override;
    std::any visitCall(struct AsgCall* node) override;
    std::any visitIntLiteral(struct AsgIntLiteral* node) override;

    llvm::AllocaInst* findAlloca(const std::string& name) const;

    std::unique_ptr<llvm::LLVMContext> context_;
    std::unique_ptr<llvm::Module> module_;
    std::unique_ptr<llvm::IRBuilder<>> builder_;
    std::unique_ptr<llvm::legacy::FunctionPassManager> fpm_;

    std::deque<std::unordered_map<std::string, llvm::AllocaInst*>> scopes_;
};


#endif //TINYC_IREMITTER_H
