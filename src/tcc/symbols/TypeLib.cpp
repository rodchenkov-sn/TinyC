#include "TypeLib.h"

#include <llvm/IR/DerivedTypes.h>


static TypeLibrary* instance = nullptr;


Type::Id TypeLibrary::get(const std::string& name) const
{
    auto realName = name;
    realName.erase(std::remove(realName.begin(), realName.end(), '*'), realName.end());

    auto refCount = std::count(name.begin(), name.end(), '*');

    if (named_types_.find(realName) == named_types_.end()) {
        return nullptr;
    }

    auto type = named_types_.at(realName);

    for (auto i = 0; i < refCount; i++) {
        type = type->getRef();
    }

    return type;
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
        {
            "int",
            std::make_shared<BaseType>(
                    [](auto& ctx) { return llvm::Type::getInt32Ty(ctx); }
            )
        },
        {
            "void",
            std::make_shared<BaseType>(
                [](auto& ctx) { return llvm::Type::getVoidTy(ctx); }
            )
        }
    });
}
