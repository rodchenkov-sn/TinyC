#ifndef TINYC_IREMITTER_H
#define TINYC_IREMITTER_H


#include <vector>
#include <sstream>
#include <stack>
#include <unordered_map>

#include "asg/AsgVisitor.h"
#include "asg/AsgNode.h"

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Utils.h"


class IrEmitter : private AsgVisitorBase {
public:
    std::unique_ptr<llvm::Module> emmit(AsgNode* root, std::string_view moduleName);

private:
    std::any visitStatementList(struct AsgStatementList* node) override;
    std::any visitFunctionDefinition(struct AsgFunctionDefinition* node) override;
    std::any visitVariableDefinition(struct AsgVariableDefinition* node) override;
    std::any visitReturn(struct AsgReturn* node) override;
    std::any visitAssignment(struct AsgAssignment* node) override;
    std::any visitAddSub(struct AsgAddSub* node) override;
    std::any visitMulDiv(struct AsgMulDiv* node) override;
    std::any visitVariable(struct AsgVariable* node) override;
    std::any visitCall(struct AsgCall* node) override;
    std::any visitIntLiteral(struct AsgIntLiteral* node) override;

    std::unique_ptr<llvm::LLVMContext> context_;
    std::unique_ptr<llvm::Module> module_;
    std::unique_ptr<llvm::IRBuilder<>> builder_;
    std::unique_ptr<llvm::legacy::FunctionPassManager> fpm_;

    std::stack<std::unordered_map<std::string, llvm::AllocaInst*>> local_variables_;
};


#endif //TINYC_IREMITTER_H
