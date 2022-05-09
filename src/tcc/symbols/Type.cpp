#include "Type.h"


ArrayType::ArrayType(Type::Id underlying, int size)
        : underlying_(std::move(underlying))
        , size_(size)
{}


Type::Id ArrayType::getRef() const
{
    return std::make_shared<PtrType>(shared_from_this());
}


Type::Id ArrayType::getArray(int size) const
{
    return std::make_shared<ArrayType>(shared_from_this(), size);
}


bool ArrayType::isPtr() const
{
    return false;
}


Type::Id ArrayType::getDeref() const
{
    return invalid();
}


bool ArrayType::isArray() const
{
    return true;
}


Type::Id ArrayType::getIndexed() const
{
    return underlying_;
}


int ArrayType::getSize() const
{
    return size_;
}


Type::Id ArrayType::getNamed() const
{
    return underlying_->getNamed();
}


llvm::Type* ArrayType::getLLVMType(llvm::LLVMContext& ctx, unsigned int addrSpace) const
{
    return llvm::ArrayType::get(underlying_->getLLVMType(ctx, addrSpace), size_);
}


llvm::Type* ArrayType::getLLVMParamType(llvm::LLVMContext& ctx, unsigned int addrSpace) const
{
    return llvm::PointerType::get(ctx, addrSpace);
}


PtrType::PtrType(Type::Id underlying)
        : underlying_(std::move(underlying))
{}

Type::Id PtrType::getRef() const
{
    return std::make_shared<PtrType>(shared_from_this());
}


Type::Id PtrType::getArray(int size) const
{
    return std::make_shared<ArrayType>(shared_from_this(), size);
}


bool PtrType::isPtr() const
{
    return true;
}


Type::Id PtrType::getDeref() const
{
    return underlying_;
}


bool PtrType::isArray() const
{
    return false;
}

Type::Id PtrType::getIndexed() const
{
    return invalid();
}


int PtrType::getSize() const
{
    return 1;
}


Type::Id PtrType::getNamed() const
{
    return underlying_->getNamed();
}


llvm::Type* PtrType::getLLVMType(llvm::LLVMContext& ctx, unsigned int addrSpace) const
{
    return llvm::PointerType::get(ctx, addrSpace);
}


BaseType::BaseType(BaseType::TypeGetter typeGetter)
        : type_getter_(std::move(typeGetter))
{}


Type::Id BaseType::getRef() const
{
    return std::make_shared<PtrType>(shared_from_this());
}


Type::Id BaseType::getArray(int size) const
{
    return std::make_shared<ArrayType>(shared_from_this(), size);
}


bool BaseType::isPtr() const
{
    return false;
}


Type::Id BaseType::getDeref() const
{
    return invalid();
}


bool BaseType::isArray() const
{
    return false;
}


Type::Id BaseType::getIndexed() const
{
    return invalid();
}


int BaseType::getSize() const
{
    return 1;
}


Type::Id BaseType::getNamed() const
{
    return shared_from_this();
}


llvm::Type* BaseType::getLLVMType(llvm::LLVMContext& ctx, unsigned int) const
{
    return type_getter_(ctx);
}
