#include "IrEmitter.h"

#include <llvm/Transforms/Utils.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/IR/Instructions.h>


std::unique_ptr<llvm::Module> IrEmitter::emmit(AsgNode* root, std::string_view moduleName, bool optimize)
{
    context_ = std::make_unique<llvm::LLVMContext>();
    module_ = std::make_unique<llvm::Module>(moduleName, *context_);
    builder_ = std::make_unique<llvm::IRBuilder<>>(*context_);

    fpm_ = std::make_unique<llvm::legacy::FunctionPassManager>(module_.get());

    if (optimize) {
        fpm_->add(llvm::createPromoteMemoryToRegisterPass());
        fpm_->add(llvm::createInstructionCombiningPass());
        fpm_->add(llvm::createReassociatePass());
        fpm_->add(llvm::createGVNPass());
        fpm_->add(llvm::createCFGSimplificationPass());
    }

    fpm_->doInitialization();

    root->accept(this);

    std::unique_ptr<llvm::Module> r;
    r.swap(module_);

    return r;
}


std::any IrEmitter::visitStatementList(struct AsgStatementList* node)
{
    if (node->parent) {
        scopes_.emplace_back();
    }

    for (auto& statement : node->statements) {
        statement->accept(this);
    }

    if (node->parent) {
        scopes_.pop_back();
    }

    return {};
}


std::any IrEmitter::visitFunctionDefinition(struct AsgFunctionDefinition* node)
{
    scopes_.emplace_back();

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

    curr_function_ = function;

    llvm::BasicBlock* bb = llvm::BasicBlock::Create(*context_, "entry", function);
    builder_->SetInsertPoint(bb);

    {
        auto i = 0;
        for (auto& param: function->args()) {
            auto& paramName = node->parameters[i++].name;

            param.setName(paramName + "_arg");

            llvm::AllocaInst* alloca = makeAlloca(paramName, param.getType());
            builder_->CreateStore(&param, alloca);

            scopes_.back().insert({ paramName, alloca });
        }
    }

    node->body->accept(this);

    assert(!llvm::verifyFunction(*function, &llvm::errs()));

    scopes_.pop_back();

    fpm_->run(*function);

    curr_function_ = nullptr;

    return {};
}


std::any IrEmitter::visitVariableDefinition(struct AsgVariableDefinition* node)
{
    auto* varType = node->list->localVariables[node->name];

    llvm::AllocaInst* alloca = makeAlloca(node->name, varType->irTypeGetter(*context_));

    scopes_.back().insert({ node->name, alloca });

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
    auto* alloca = findAlloca(node->name);
    auto* value = std::any_cast<llvm::Value*>(node->value->accept(this));

    builder_->CreateStore(value, alloca);

    return {};
}


std::any IrEmitter::visitConditional(struct AsgConditional* node)
{
    auto* condVal = std::any_cast<llvm::Value*>(node->condition->accept(this));
    auto* constZero = llvm::ConstantInt::getSigned(llvm::Type::getInt32Ty(*context_), 0);
    auto* cond = builder_->CreateICmpNE(condVal, constZero);

    llvm::BasicBlock* thenBB = llvm::BasicBlock::Create(*context_, "then", curr_function_);

    llvm::BasicBlock* elseBB = nullptr;
    if (node->elseNode) {
        elseBB = llvm::BasicBlock::Create(*context_, "else", curr_function_);
    }

    llvm::BasicBlock* mergeBB = nullptr;

    auto* parentList = dynamic_cast<AsgStatementList*>(node->parent);
    const auto isLastNode = !parentList || parentList->statements.back().get() == node;

    if (!isLastNode) {
        mergeBB = llvm::BasicBlock::Create(*context_, "merge", curr_function_);
    }

    if (elseBB) {
        builder_->CreateCondBr(cond, thenBB, elseBB);
    } else {
        builder_->CreateCondBr(cond, thenBB, mergeBB);
    }

    {
        builder_->SetInsertPoint(thenBB);
        node->thenNode->accept(this);
        builder_->SetInsertPoint(thenBB);
        const auto endedWithRet = thenBB->back().isTerminator();
        if (!isLastNode && !endedWithRet) {
            builder_->CreateBr(mergeBB);
        }
    }

    if (elseBB) {
        builder_->SetInsertPoint(elseBB);
        node->elseNode->accept(this);
        builder_->SetInsertPoint(elseBB);
        const auto endedWithRet = elseBB->back().isTerminator();
        if (!isLastNode && !endedWithRet) {
            builder_->CreateBr(mergeBB);
        }
    }

    if (mergeBB) {
        builder_->SetInsertPoint(mergeBB);
    }

    return {};
}


std::any IrEmitter::visitComp(struct AsgComp* node)
{
    auto* lhs = std::any_cast<llvm::Value*>(node->lhs->accept(this));
    auto* rhs = std::any_cast<llvm::Value*>(node->rhs->accept(this));

    llvm::Value* i1Val = nullptr;

    switch (node->op) {
        case AsgComp::Operator::Equals:
            i1Val = builder_->CreateICmpEQ(lhs, rhs);
            break;
        case AsgComp::Operator::NotEquals:
            i1Val = builder_->CreateICmpNE(lhs, rhs);
            break;
        case AsgComp::Operator::Less:
            i1Val = builder_->CreateICmpSLT(lhs, rhs);
            break;
        case AsgComp::Operator::LessEquals:
            i1Val = builder_->CreateICmpSLE(lhs, rhs);
            break;
        case AsgComp::Operator::Greater:
            i1Val = builder_->CreateICmpSGT(lhs, rhs);
            break;
        case AsgComp::Operator::GreaterEquals:
            i1Val = builder_->CreateICmpSGE(lhs, rhs);
            break;
    }

    return builder_->CreateIntCast(i1Val, llvm::Type::getInt32Ty(*context_), false);
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
    auto* alloca = findAlloca(node->name);
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

llvm::AllocaInst* IrEmitter::findAlloca(const std::string& name) const
{
    for (auto scope = scopes_.rbegin(); scope != scopes_.rend(); scope++) {
        if (scope->find(name) != scope->end()) {
            return scope->at(name);
        }
    }
    return nullptr;
}

llvm::AllocaInst* IrEmitter::makeAlloca(const std::string& name, llvm::Type* type)
{
    llvm::IRBuilder<> builder{
        &curr_function_->getEntryBlock(),
        curr_function_->getEntryBlock().begin()
    };

    return builder.CreateAlloca(type, nullptr, name);
}
