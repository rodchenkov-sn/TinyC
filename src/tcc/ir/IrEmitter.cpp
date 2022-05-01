#include "IrEmitter.h"

#include <llvm/Transforms/Utils.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>


std::unique_ptr<llvm::Module> IrEmitter::emmit(AsgNode* root, std::string_view moduleName)
{
    context_ = std::make_unique<llvm::LLVMContext>();
    module_ = std::make_unique<llvm::Module>(moduleName, *context_);
    builder_ = std::make_unique<llvm::IRBuilder<>>(*context_);

    fpm_ = std::make_unique<llvm::legacy::FunctionPassManager>(module_.get());

    fpm_->add(llvm::createPromoteMemoryToRegisterPass());
    fpm_->add(llvm::createInstructionCombiningPass());
    fpm_->add(llvm::createReassociatePass());
    fpm_->add(llvm::createGVNPass());
    fpm_->add(llvm::createCFGSimplificationPass());

    fpm_->doInitialization();

    root->accept(this);

    std::unique_ptr<llvm::Module> r;
    r.swap(module_);

    return r;
}


std::any IrEmitter::visitStatementList(struct AsgStatementList* node)
{
    for (auto& statement : node->statements) {
        statement->accept(this);
    }

    return {};
}


std::any IrEmitter::visitFunctionDefinition(struct AsgFunctionDefinition* node)
{
    local_variables_.emplace();

    std::vector<llvm::Type*> paramTypes;
    for (auto& paramType : node->type->parameters) {
        paramTypes.push_back(paramType->irTypeGetter(*context_));
    }

    llvm::FunctionType* functionType = llvm::FunctionType::get(
        node->type->returnType->irTypeGetter(*context_),
        paramTypes,
        false
    );

    llvm::Function* function = llvm::Function::Create(
        functionType,
        llvm::Function::ExternalLinkage,
        node->name,
        module_.get()
    );

    llvm::BasicBlock* bb = llvm::BasicBlock::Create(*context_, "entry", function);
    builder_->SetInsertPoint(bb);

    {
        auto i = 0;
        for (auto& param: function->args()) {
            auto& paramName = node->parameters[i++].name;

            param.setName(paramName + "_arg");

            llvm::AllocaInst* alloca = builder_->CreateAlloca(param.getType(), nullptr, paramName);
            builder_->CreateStore(&param, alloca);

            local_variables_.top().insert({ paramName, alloca });
        }
    }

    node->body->accept(this);

    assert(!llvm::verifyFunction(*function, &llvm::errs()));

    local_variables_.pop();

    fpm_->run(*function);

    return {};
}


std::any IrEmitter::visitVariableDefinition(struct AsgVariableDefinition* node)
{
    auto* varType = node->parent->localVariables[node->name];

    llvm::AllocaInst* alloca = builder_->CreateAlloca(varType->irTypeGetter(*context_), nullptr, node->name);

    local_variables_.top().insert({ node->name, alloca });

    if (node->value) {
        auto* value = std::any_cast<llvm::Value*>(node->value->accept(this));
        builder_->CreateStore(value, alloca);
    }

    return {};
}


std::any IrEmitter::visitReturn(struct AsgReturn* node)
{
    auto* value = std::any_cast<llvm::Value*>(node->value->accept(this));

    builder_->CreateRet(value);

    return {};
}


std::any IrEmitter::visitAssignment(struct AsgAssignment* node)
{
    auto* alloca = local_variables_.top()[node->name];
    auto* value = std::any_cast<llvm::Value*>(node->value->accept(this));

    builder_->CreateStore(value, alloca);

    return {};
}


std::any IrEmitter::visitAddSub(struct AsgAddSub* node)
{
    auto* last = std::any_cast<llvm::Value*>(node->subexpressions[0].expression->accept(this));
    for (auto i = 1; i < node->subexpressions.size(); i++) {
        auto* curr = std::any_cast<llvm::Value*>(node->subexpressions[i].expression->accept(this));
        if (node->subexpressions[i].leadingOp == AsgAddSub::Operator::Add) {
            last = builder_->CreateAdd(last, curr);
        } else {
            last = builder_->CreateSub(last, curr);
        }
    }

    return (llvm::Value*)last;
}


std::any IrEmitter::visitMulDiv(struct AsgMulDiv* node)
{
    auto* last = std::any_cast<llvm::Value*>(node->subexpressions[0].expression->accept(this));
    for (auto i = 1; i < node->subexpressions.size(); i++) {
        auto* curr = std::any_cast<llvm::Value*>(node->subexpressions[i].expression->accept(this));
        if (node->subexpressions[i].leadingOp == AsgMulDiv::Operator::Mul) {
            last = builder_->CreateMul(last, curr);
        } else {
            last = builder_->CreateSDiv(last, curr);
        }
    }

    return (llvm::Value*)last;
}


std::any IrEmitter::visitVariable(struct AsgVariable* node)
{
    auto* alloca = local_variables_.top()[node->name];
    return (llvm::Value*)builder_->CreateLoad(alloca);
}


std::any IrEmitter::visitCall(struct AsgCall* node)
{
    auto* callee = module_->getFunction(node->functionName);

    std::vector<llvm::Value*> args;
    for (auto& expr : node->arguments) {
        args.push_back(std::any_cast<llvm::Value*>(expr->accept(this)));
    }

    return (llvm::Value*)builder_->CreateCall(callee, args);
}


std::any IrEmitter::visitIntLiteral(struct AsgIntLiteral* node)
{
    return (llvm::Value*)llvm::ConstantInt::getSigned(llvm::Type::getInt32Ty(*context_), node->value);
}
