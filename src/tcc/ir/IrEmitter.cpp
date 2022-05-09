#include "IrEmitter.h"

#include <llvm/Transforms/Utils.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>

#include "symbols/TypeLib.h"


std::unique_ptr<llvm::Module> IrEmitter::emit(AsgNode* root, std::string_view moduleName, bool optimize)
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
    op_context_.push(Context::Undefined);
    scopes_.emplace_back();

    std::vector<llvm::Type*> paramTypes;
    for (auto& paramType : node->type->parameters) {
        paramTypes.push_back(paramType->getLLVMParamType(*context_, 0));
    }

    llvm::FunctionType* functionType = llvm::FunctionType::get(
        node->type->returnType->getLLVMType(*context_, 0),
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

    if (node->type->returnType == TypeLibrary::inst().get("void")) {
        // ToDo: kill it with fire
        auto* topLevelList = (AsgStatementList*)node->body.get();
        topLevelList->statements.push_back(std::make_unique<AsgReturn>());
    }

    node->body->accept(this);

    llvm::verifyFunction(*function, &llvm::errs());

    scopes_.pop_back();

    fpm_->run(*function);

    curr_function_ = nullptr;
    op_context_.pop();
    return {};
}


std::any IrEmitter::visitVariableDefinition(struct AsgVariableDefinition* node)
{
    auto varType = node->list->localVariables[node->name];

    llvm::AllocaInst* alloca = makeAlloca(node->name, varType->getLLVMType(*context_, curr_function_->getAddressSpace()));

    scopes_.back().insert({ node->name, alloca });

    if (node->value) {
        if (type_calculator_.calculate(node->value.get()) != varType) {
            std::cerr << "invalid type conversion at " << node->name << " def\n";
        }
        op_context_.push(Context::Load);
        auto* value = std::any_cast<llvm::Value*>(node->value->accept(this));
        op_context_.pop();
        alloca->mutateType(llvm::PointerType::get(*context_, curr_function_->getAddressSpace()));
        builder_->CreateStore(value, alloca);
    }

    return (llvm::Value*)builder_->CreateLoad(varType->getLLVMType(*context_, curr_function_->getAddressSpace()), alloca);
}


std::any IrEmitter::visitReturn(struct AsgReturn* node)
{
    if (!node->value) {
        builder_->CreateRetVoid();
        return {};
    }

    op_context_.push(Context::Load);
    auto* value = std::any_cast<llvm::Value*>(node->value->accept(this));
    op_context_.pop();
    auto retType = type_calculator_.calculate(node->value.get());

    if (retType != node->function->type->returnType) {
        std::cerr << "invalid return type in " << node->function->name << "\n";
    }

    builder_->CreateRet(value);

    return {};
}


std::any IrEmitter::visitAssignment(struct AsgAssignment* node)
{
    op_context_.push(Context::Load);
    auto* value = std::any_cast<llvm::Value*>(node->value->accept(this));
    op_context_.pop();
    auto valType = type_calculator_.calculate(node->value.get());
    op_context_.push(Context::NoLoad);
    auto* assignable = std::any_cast<llvm::Value*>(node->assignable->accept(this));
    op_context_.pop();
    auto assignableType = type_calculator_.calculate(node->assignable.get());

    assignable->mutateType(llvm::PointerType::get(*context_, curr_function_->getAddressSpace()));
    builder_->CreateStore(value, assignable);

    return (llvm::Value*)builder_->CreateLoad(
        assignableType->getLLVMType(*context_, curr_function_->getAddressSpace()), assignable
    );
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


std::any IrEmitter::visitLoop(struct AsgLoop* node)
{
    llvm::BasicBlock* condBlock = llvm::BasicBlock::Create(*context_, "loop_cond", curr_function_);
    llvm::BasicBlock* loopBlock = llvm::BasicBlock::Create(*context_, "loop_body", curr_function_);
    llvm::BasicBlock* mergeBlock = llvm::BasicBlock::Create(*context_, "loop_merge", curr_function_);

    builder_->CreateBr(condBlock);

    builder_->SetInsertPoint(condBlock);
    auto* condVal = std::any_cast<llvm::Value*>(node->condition->accept(this));
    auto* constZero = llvm::ConstantInt::getSigned(llvm::Type::getInt32Ty(*context_), 0);
    auto* cond = builder_->CreateICmpNE(condVal, constZero);
    builder_->CreateCondBr(cond, loopBlock, mergeBlock);

    builder_->SetInsertPoint(loopBlock);
    node->body->accept(this);
    builder_->SetInsertPoint(loopBlock);
    builder_->CreateBr(condBlock);

    builder_->SetInsertPoint(mergeBlock);

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
    op_context_.push(Context::Load);
    auto* last = std::any_cast<llvm::Value*>(node->subexpressions[0].expression->accept(this));
    for (auto i = 1; i < node->subexpressions.size(); i++) {
        auto* curr = std::any_cast<llvm::Value*>(node->subexpressions[i].expression->accept(this));
        if (node->subexpressions[i].leadingOp == AsgAddSub::Operator::Add) {
            last = builder_->CreateAdd(last, curr);
        } else {
            last = builder_->CreateSub(last, curr);
        }
    }
    op_context_.pop();
    return (llvm::Value*)last;
}


std::any IrEmitter::visitMulDiv(struct AsgMulDiv* node)
{
    op_context_.push(Context::Load);
    auto* last = std::any_cast<llvm::Value*>(node->subexpressions[0].expression->accept(this));
    for (auto i = 1; i < node->subexpressions.size(); i++) {
        auto* curr = std::any_cast<llvm::Value*>(node->subexpressions[i].expression->accept(this));
        if (node->subexpressions[i].leadingOp == AsgMulDiv::Operator::Mul) {
            last = builder_->CreateMul(last, curr);
        } else {
            last = builder_->CreateSDiv(last, curr);
        }
    }
    op_context_.pop();
    return (llvm::Value*)last;
}


std::any IrEmitter::visitIndexing(struct AsgIndexing* node)
{
    op_context_.push(Context::NoLoad);
    auto* indexed = std::any_cast<llvm::Value*>(node->indexed->accept(this));
    op_context_.pop();
    indexed->mutateType(llvm::PointerType::get(*context_, curr_function_->getAddressSpace()));
    auto indexedType = type_calculator_.calculate(node->indexed.get());

    op_context_.push(Context::Load);

    std::array<llvm::Value*, 2> currGepIdx{};
    currGepIdx[0] = llvm::ConstantInt::getSigned(llvm::Type::getInt64Ty(*context_), 0);
    llvm::Value* lastGEP = nullptr;

    if (!indexedType->isArray()) {
        std::cerr << "invalid indexing\n";
    }

    if (indexedType->getSize() == -1) {
        lastGEP = builder_->CreateGEP(
            indexedType->getIndexed()->getLLVMType(*context_, curr_function_->getAddressSpace()),
            builder_->CreateLoad(
                llvm::PointerType::get(*context_, curr_function_->getAddressSpace()),
                indexed
            ),
            std::any_cast<llvm::Value*>(node->indexes[0]->accept(this))
        );
    } else {
        currGepIdx[1] = std::any_cast<llvm::Value*>(node->indexes[0]->accept(this));
        lastGEP = builder_->CreateInBoundsGEP(
            indexedType->getLLVMType(*context_, curr_function_->getAddressSpace()),
            indexed,
            currGepIdx
        );
    }

    indexedType = indexedType->getIndexed();

    for (auto i = 1; i < node->indexes.size(); i++) {
        if (!indexedType) {
            std::cerr << "invalid indexing\n";
        }
        currGepIdx[1] = std::any_cast<llvm::Value*>(node->indexes[i]->accept(this));
        lastGEP = builder_->CreateGEP(
            indexedType->getLLVMType(*context_, curr_function_->getAddressSpace()),
            lastGEP,
            currGepIdx
        );
        indexedType = indexedType->getIndexed();
    }

    op_context_.pop();

    if (op_context_.top() == Context::NoLoad) {
        return lastGEP;
    }
    return (llvm::Value*)builder_->CreateLoad(
        indexedType->getLLVMType(*context_, curr_function_->getAddressSpace()),
        lastGEP
    );
}


std::any IrEmitter::visitOpDeref(struct AsgOpDeref* node)
{
    auto derefCount = node->derefCount;

    if (op_context_.top() == Context::NoLoad) {
        derefCount--;
    }

    auto* lastLoad = std::any_cast<llvm::Value*>(node->expression->accept(this));
    auto exprType = type_calculator_.calculate(node->expression.get());

    for (auto i = 0; i < derefCount; i++) {
        auto derefType = exprType->getDeref();
        if (!derefType) {
            std::cerr << "invalid dereference\n";
        }
        lastLoad = builder_->CreateLoad(
            derefType->getLLVMType(*context_, curr_function_->getAddressSpace()),
            lastLoad
        );
        exprType = derefType;
    }
    return lastLoad;
}


std::any IrEmitter::visitOpRef(struct AsgOpRef* node)
{
    llvm::Value* val = nullptr;

    if (auto* variable = dynamic_cast<AsgVariable*>(node->value.get())) {
        val = findAlloca(variable->name);
    } else {
        std::cerr << "unexpected reference in " << node->function->name << "\n";
    }

    auto valType = type_calculator_.calculate(node);
    return val;
}


std::any IrEmitter::visitVariable(struct AsgVariable* node)
{
    if (op_context_.top() == Context::NoLoad) {
        return (llvm::Value*)findAlloca(node->name);
    } else if (const auto varType = findVarType(node->name, node); varType->isArray() && op_context_.top() == Context::Call) {
        auto* alloca = findAlloca(node->name);
        alloca->mutateType(llvm::PointerType::get(*context_, curr_function_->getAddressSpace()));
        auto* constZero = llvm::ConstantInt::getSigned(llvm::Type::getInt64Ty(*context_), 0);
        std::vector<llvm::Value*> ids;
        ids.push_back(constZero);
        auto currType = varType;
        while (currType->isArray()) {
            ids.push_back(constZero);
            currType = varType->getIndexed();
        }
        return (llvm::Value*)builder_->CreateInBoundsGEP(
            varType->getLLVMType(*context_, curr_function_->getAddressSpace()),
            alloca,
            ids
        );
    }
    return (llvm::Value*)builder_->CreateLoad(
        findVarType(node->name, node)->getLLVMType(*context_, curr_function_->getAddressSpace()),
        findAlloca(node->name)
    );
}


std::any IrEmitter::visitCall(struct AsgCall* node)
{
    auto* callee = module_->getFunction(node->functionName);

    op_context_.push(Context::Call);
    std::vector<llvm::Value*> args;
    for (auto& expr : node->arguments) {
        args.push_back(std::any_cast<llvm::Value*>(expr->accept(this)));
    }
    op_context_.pop();

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


Type::Id IrEmitter::findVarType(const std::string& name, const AsgNode* startNode)
{
    const AsgStatementList* list = startNode->list;
    while (list) {
        if (list->localVariables.find(name) != list->localVariables.end()) {
            return list->localVariables.at(name);
        }
        list = list->list;
    }
    return nullptr;
}


std::any IrEmitter::TypeCalculator::visitStatementList(struct AsgStatementList* node)
{
    return Type::invalid();
}


std::any IrEmitter::TypeCalculator::visitFunctionDefinition(struct AsgFunctionDefinition* node)
{
    return Type::invalid();
}


std::any IrEmitter::TypeCalculator::visitVariableDefinition(struct AsgVariableDefinition* node)
{
    return findVarType(node->name, node);
}


std::any IrEmitter::TypeCalculator::visitReturn(struct AsgReturn* node)
{
    return Type::invalid();
}


std::any IrEmitter::TypeCalculator::visitAssignment(struct AsgAssignment* node)
{
    return node->assignable->accept(this);
}


std::any IrEmitter::TypeCalculator::visitConditional(struct AsgConditional* node)
{
    return Type::invalid();
}


std::any IrEmitter::TypeCalculator::visitLoop(struct AsgLoop* node)
{
    return Type::invalid();
}


std::any IrEmitter::TypeCalculator::visitComp(struct AsgComp* node)
{
    auto lt = std::any_cast<Type::Id>(node->lhs->accept(this));
    auto rt = std::any_cast<Type::Id>(node->rhs->accept(this));
    if (lt && rt) {
        return TypeLibrary::inst().get("int");
    }
    return Type::invalid();
}


std::any IrEmitter::TypeCalculator::visitAddSub(struct AsgAddSub* node)
{
    for (auto& sub : node->subexpressions) {
        auto subtype = std::any_cast<Type::Id>(sub.expression->accept(this));
        if (!subtype || subtype != TypeLibrary::inst().get("int")) {
            return Type::invalid();
        }
    }
    return TypeLibrary::inst().get("int");
}


std::any IrEmitter::TypeCalculator::visitMulDiv(struct AsgMulDiv* node)
{
    for (auto& sub : node->subexpressions) {
        auto subtype = std::any_cast<Type::Id>(sub.expression->accept(this));
        if (!subtype || subtype != TypeLibrary::inst().get("int")) {
            return Type::invalid();
        }
    }
    return TypeLibrary::inst().get("int");
}


std::any IrEmitter::TypeCalculator::visitIndexing(struct AsgIndexing* node)
{
    for (auto& index : node->indexes) {
        if (std::any_cast<Type::Id>(index->accept(this)) != TypeLibrary::inst().get("int")) {
            return Type::invalid();
        }
    }

    auto indexedType = std::any_cast<Type::Id>(node->indexed->accept(this));

    if (!indexedType) {
        return indexedType;
    }

    for (auto& index : node->indexes) {
        if (indexedType->isArray()) {
            indexedType = indexedType->getIndexed();
        } else if (indexedType->isPtr()) {
            indexedType = indexedType->getDeref();
        } else {
            indexedType = Type::invalid();
            break;
        }
    }

    return indexedType;
}


std::any IrEmitter::TypeCalculator::visitOpDeref(struct AsgOpDeref* node)
{
    auto derefCount = node->derefCount;

    auto expType = std::any_cast<Type::Id>(node->expression->accept(this));

    if (!expType->isPtr()) {
        return Type::invalid();
    }

    for (int i = 0; i < derefCount; i++) {
        if (expType) {
            expType = expType->getDeref();
        }
    }

    return expType;
}


std::any IrEmitter::TypeCalculator::visitOpRef(struct AsgOpRef* node)
{
    auto expType = std::any_cast<Type::Id>(node->value->accept(this));

    if (!expType) {
        return expType;
    }

    return expType->getRef();
}


std::any IrEmitter::TypeCalculator::visitVariable(struct AsgVariable* node)
{
    return IrEmitter::findVarType(node->name, node);
}


std::any IrEmitter::TypeCalculator::visitCall(struct AsgCall* node)
{
    return node->callee->returnType;
}


std::any IrEmitter::TypeCalculator::visitIntLiteral(struct AsgIntLiteral* node)
{
    return TypeLibrary::inst().get("int");
}


Type::Id IrEmitter::TypeCalculator::calculate(AsgNode* node)
{
    return std::any_cast<Type::Id>(node->accept(this));
}
