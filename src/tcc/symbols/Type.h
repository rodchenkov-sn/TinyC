#ifndef TINYC_TCTYPE_H
#define TINYC_TCTYPE_H

#include <functional>
#include <utility>

#include <llvm/IR/Type.h>

#define DECL_TYPE_CATEGORY(c)                   \
    static Type::Category getCategoryStatic()   \
    {                                           \
        return c;                               \
    }                                           \
    Type::Category getCategory() const override \
    {                                           \
        return c;                               \
    }

struct Type : public std::enable_shared_from_this<Type> {
    using Id = std::shared_ptr<Type>;

    enum class Category {
        Basic,
        Ptr,
        Array,
        Struct
    };

    static Id invalid()
    {
        return nullptr;
    };

    static bool isSame(const Type::Id& lhs, const Type::Id& rhs);

    template<typename T>
    T* as()
    {
        if (getCategory() == T::getCategoryStatic()) {
            return static_cast<T*>(this);
        }
        return nullptr;
    }

    virtual Id getRef();
    virtual Id getArray(int size);

    virtual Category getCategory() const = 0;
    virtual Id getNamed() = 0;

    virtual std::string toString() = 0;

    virtual llvm::Type* getLLVMType(llvm::LLVMContext& ctx, unsigned int addrSpace) const = 0;
    virtual llvm::Type* getLLVMParamType(llvm::LLVMContext& ctx, unsigned int addrSpace) const
    {
        return getLLVMType(ctx, addrSpace);
    }
};

class StructType : public Type {
public:
    DECL_TYPE_CATEGORY(Category::Struct)

    explicit StructType(std::string name, std::vector<std::pair<Id, std::string>> fields);

    int getFieldId(std::string_view name) const;
    Id getFieldType(std::string_view name) const;

    Id getNamed() override;

    std::string toString() override;

    llvm::Type* getLLVMType(llvm::LLVMContext& ctx, unsigned int addrSpace) const override;
    llvm::Type* getLLVMParamType(llvm::LLVMContext& ctx, unsigned int addrSpace) const override;

private:
    mutable llvm::Type* self_type_ = nullptr;
    std::string name_;
    std::vector<std::pair<Id, std::string>> fields_;
};

class ArrayType : public Type {
public:
    DECL_TYPE_CATEGORY(Category::Array)

    ArrayType(Id underlying, int size);

    Id getIndexed() const;
    int getSize() const;

    Id getNamed() override;

    std::string toString() override;

    llvm::Type* getLLVMType(llvm::LLVMContext& ctx, unsigned int addrSpace) const override;
    llvm::Type* getLLVMParamType(llvm::LLVMContext& ctx, unsigned int addrSpace) const override;

private:
    Id underlying_;
    int size_;
};

class PtrType : public Type {
public:
    DECL_TYPE_CATEGORY(Category::Ptr)

    explicit PtrType(Id underlying);

    Id getDeref() const;

    Id getNamed() override;

    std::string toString() override;

    llvm::Type* getLLVMType(llvm::LLVMContext& ctx, unsigned int addrSpace) const override;

private:
    Id underlying_;
};

class BaseType : public Type {
public:
    using TypeGetter = std::function<llvm::Type*(llvm::LLVMContext&)>;

    DECL_TYPE_CATEGORY(Category::Basic);

    explicit BaseType(std::string name, TypeGetter typeGetter);

    Id getNamed() override;

    std::string toString() override;

    llvm::Type* getLLVMType(llvm::LLVMContext& ctx, unsigned int) const override;

private:
    std::string name_;
    TypeGetter type_getter_;
};

struct LRValue {
    enum class Side {
        L,
        R
    };

    explicit LRValue(Type::Id t, Side s = Side::R)
        : type(std::move(t))
        , side(s)
    {
    }

    Type::Id type;
    Side side;
};

#undef DECL_TYPE_CATEGORY

#endif
