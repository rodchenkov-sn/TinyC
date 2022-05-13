#ifndef TINYC_IREMITTER_H
#define TINYC_IREMITTER_H

#include <deque>
#include <sstream>
#include <stack>
#include <unordered_map>
#include <vector>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>

#include "asg/AsgNode.h"
#include "asg/AsgVisitor.h"

class IrEmitter : private AsgVisitorBase {
public:
    std::unique_ptr<llvm::Module> emit(AsgNode* root, std::string_view moduleName, bool optimize = true);

private:
    class TypeCalculator : private AsgVisitorBase {
    public:
        Type::Id calculate(AsgNode* node);

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
    };

    enum class RetType {
        Ptr,
        Data,
        CallParam,
        Undef
    };

    friend class TypeCalculator;

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

    llvm::AllocaInst* findAlloca(const std::string& name) const;
    llvm::AllocaInst* makeAlloca(const std::string& name, llvm::Type* type);

    static Type::Id findVarType(const std::string& name, const AsgNode* startNode);

    std::unique_ptr<llvm::LLVMContext> context_;
    std::unique_ptr<llvm::Module> module_;
    std::unique_ptr<llvm::IRBuilder<>> builder_;

    std::deque<std::unordered_map<std::string, llvm::AllocaInst*>> scopes_;
    llvm::Function* curr_function_;

    TypeCalculator type_calculator_;
    std::stack<RetType> expected_ret_;
};

#endif
