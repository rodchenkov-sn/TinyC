#ifndef TINYC_TCTYPE_H
#define TINYC_TCTYPE_H

#include <functional>
#include <utility>

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Type.h>

struct Type {
    using Id = std::shared_ptr<const Type>;

    static Id invalid()
    {
        return nullptr;
    };

    virtual Id getRef() const = 0;
    virtual Id getArray(int size) const = 0;

    virtual bool isPtr() const = 0;
    virtual Id getDeref() const = 0;

    virtual bool isArray() const = 0;
    virtual Id getIndexed() const = 0;
    virtual int getSize() const = 0;

    virtual Id getNamed() const = 0;

    virtual llvm::Type* getLLVMType(llvm::LLVMContext& ctx, unsigned int addrSpace) const = 0;
    virtual llvm::Type* getLLVMParamType(llvm::LLVMContext& ctx, unsigned int addrSpace) const
    {
        return getLLVMType(ctx, addrSpace);
    }
};

class ArrayType : public Type
    , public std::enable_shared_from_this<ArrayType> {
public:
    ArrayType(Id underlying, int size);

    Id getRef() const override;
    Id getArray(int size) const override;

    bool isPtr() const override;
    Id getDeref() const override;

    bool isArray() const override;
    Id getIndexed() const override;
    int getSize() const override;

    Id getNamed() const override;

    llvm::Type* getLLVMType(llvm::LLVMContext& ctx, unsigned int addrSpace) const override;
    llvm::Type* getLLVMParamType(llvm::LLVMContext& ctx, unsigned int addrSpace) const override;

private:
    Id underlying_;
    int size_;
};

class PtrType : public Type
    , public std::enable_shared_from_this<PtrType> {
public:
    explicit PtrType(Id underlying);

    Id getRef() const override;
    Id getArray(int size) const override;

    bool isPtr() const override;
    Id getDeref() const override;

    bool isArray() const override;
    Id getIndexed() const override;
    int getSize() const override;

    Id getNamed() const override;

    llvm::Type* getLLVMType(llvm::LLVMContext& ctx, unsigned int addrSpace) const override;

private:
    Id underlying_;
};

class BaseType : public Type
    , public std::enable_shared_from_this<BaseType> {
    using TypeGetter = std::function<llvm::Type*(llvm::LLVMContext&)>;

public:
    explicit BaseType(TypeGetter typeGetter);

    Id getRef() const override;
    Id getArray(int size) const override;

    bool isPtr() const override;
    Id getDeref() const override;

    bool isArray() const override;
    Id getIndexed() const override;
    int getSize() const override;

    Id getNamed() const override;

    llvm::Type* getLLVMType(llvm::LLVMContext& ctx, unsigned int) const override;

private:
    TypeGetter type_getter_;
};

#endif
