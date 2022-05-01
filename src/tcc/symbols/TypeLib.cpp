#include "TypeLib.h"

#include <llvm/IR/DerivedTypes.h>


static TypeLibrary* instance = nullptr;


const Type* TypeLibrary::get(const std::string& name) const
{
    if (named_types_.find(name) != named_types_.end()) {
        return &named_types_.at(name);
    }
    return nullptr;
}


TypeLibrary& TypeLibrary::inst()
{
    if (!instance) {
        instance = new TypeLibrary;
    }
    return *instance;
}

TypeLibrary::TypeLibrary()
{
    named_types_.insert({
        "int",
        Type{
            [](auto& ctx) { return llvm::Type::getInt32Ty(ctx); }
        }
    });
    named_types_.insert({
        "void",
        Type{
            [](auto& ctx) { return llvm::Type::getVoidTy(ctx); }
        }
    });
}
