#ifndef TINYC_TCTYPE_H
#define TINYC_TCTYPE_H


#include <functional>
#include <utility>

#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>


struct Type {
    using Id = std::shared_ptr<const Type>;

    static Id invalid() { return nullptr; };

    virtual Id getRef() const = 0;

    virtual bool isPtr() const = 0;
    virtual Id getDeref() const = 0;

    virtual Id getNamed() const = 0;

    virtual llvm::Type* getLLVMType(llvm::LLVMContext& ctx, unsigned int addrSpace) const = 0;
};


class PtrType : public Type, public std::enable_shared_from_this<PtrType> {
public:
    explicit PtrType(Id underlying)
        : underlying_(std::move(underlying))
    {}

    Id getRef() const override { return std::make_shared<PtrType>(shared_from_this()); }

    bool isPtr() const override { return true; }

    Id getDeref() const override { return underlying_; }

    Id getNamed() const override { return underlying_->getNamed(); }

    llvm::Type* getLLVMType(llvm::LLVMContext& ctx, unsigned int addrSpace) const override
    {
        return llvm::PointerType::get(ctx, addrSpace);
    }

private:
    Id underlying_;
};


class BaseType : public Type, public std::enable_shared_from_this<BaseType> {
    using TypeGetter = std::function<llvm::Type*(llvm::LLVMContext&)>;

public:
    explicit BaseType(TypeGetter typeGetter)
        : type_getter_(std::move(typeGetter))
    {}

    Id getRef() const override { return std::make_shared<PtrType>(shared_from_this()); }

    bool isPtr() const override { return false; }
    Id getDeref() const override { return nullptr; }

    Id getNamed() const override { return shared_from_this(); }

    llvm::Type* getLLVMType(llvm::LLVMContext& ctx, unsigned int) const override { return type_getter_(ctx); }

private:
    TypeGetter type_getter_;
};


#endif // TINYC_TCTYPE_H
