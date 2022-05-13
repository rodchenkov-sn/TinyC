#include "Type.h"

#include <algorithm>

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>

bool Type::isSame(const Type::Id& lhs, const Type::Id& rhs)
{
    if (!lhs || !rhs) {
        return false;
    }
    if (lhs->getCategory() != rhs->getCategory()) {
        return false;
    }
    switch (lhs->getCategory()) {
    case Category::Basic:
        return lhs == rhs;
    case Category::Ptr:
        return isSame(lhs->as<PtrType>()->getDeref(), rhs->as<PtrType>()->getDeref());
    case Category::Array: {
        auto* lhsA = lhs->as<ArrayType>();
        auto* rhsA = rhs->as<ArrayType>();
        if (lhsA->getSize() == -1 || rhsA->getSize() == -1) {
            return isSame(lhsA->getIndexed(), rhsA->getIndexed());
        }
        return isSame(lhsA->getIndexed(), rhsA->getIndexed()) && lhsA->getSize() == rhsA->getSize();
    }
    case Category::Struct:
        return lhs == rhs;
    }
}

Type::Id Type::getRef()
{
    return std::make_shared<PtrType>(shared_from_this());
}

Type::Id Type::getArray(int size)
{
    return std::make_shared<ArrayType>(shared_from_this(), size);
}

StructType::StructType(std::vector<std::pair<Id, std::string>> fields)
    : fields_(std::move(fields))
{
}

int StructType::getFieldId(std::string_view name) const
{
    auto elem = std::find_if(fields_.begin(), fields_.end(), [&](const auto& p) { return p.second == name; });
    if (elem == fields_.end()) {
        return -1;
    }
    return (int)(elem - fields_.begin());
}

Type::Id StructType::getFieldType(std::string_view name) const
{
    auto elem = std::find_if(fields_.begin(), fields_.end(), [&](const auto& p) { return p.second == name; });
    if (elem == fields_.end()) {
        return invalid();
    }
    return elem->first;
}

void StructType::create(std::string_view name, llvm::LLVMContext& ctx, unsigned int addrSpace)
{
    std::vector<llvm::Type*> types;
    std::transform(fields_.begin(), fields_.end(), std::back_inserter(types), [&](const auto& pair) {
        return pair.first->getLLVMType(ctx, addrSpace);
    });
    self_type_ = llvm::StructType::create(types, name);
}

Type::Id StructType::getNamed()
{
    return shared_from_this();
}

llvm::Type* StructType::getLLVMType(llvm::LLVMContext& ctx, unsigned int addrSpace) const
{
    return self_type_;
}

llvm::Type* StructType::getLLVMParamType(llvm::LLVMContext& ctx, unsigned int addrSpace) const
{
    return llvm::PointerType::get(ctx, addrSpace);
}

ArrayType::ArrayType(Type::Id underlying, int size)
    : underlying_(std::move(underlying))
    , size_(size)
{
}

Type::Id ArrayType::getIndexed() const
{
    return underlying_;
}

int ArrayType::getSize() const
{
    return size_;
}

Type::Id ArrayType::getNamed()
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
{
}

Type::Id PtrType::getDeref() const
{
    return underlying_;
}

Type::Id PtrType::getNamed()
{
    return underlying_->getNamed();
}

llvm::Type* PtrType::getLLVMType(llvm::LLVMContext& ctx, unsigned int addrSpace) const
{
    return llvm::PointerType::get(ctx, addrSpace);
}

BaseType::BaseType(BaseType::TypeGetter typeGetter)
    : type_getter_(std::move(typeGetter))
{
}

Type::Id BaseType::getNamed()
{
    return shared_from_this();
}

llvm::Type* BaseType::getLLVMType(llvm::LLVMContext& ctx, unsigned int) const
{
    return type_getter_(ctx);
}
