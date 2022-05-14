#include "TypeLib.h"

#include <regex>

#include <llvm/IR/DerivedTypes.h>

static TypeLibrary* instance = nullptr;

Type::Id TypeLibrary::get(const std::string& name) const
{
    static const std::regex rIndex{R"(\[(\d*)\])"};

    auto realName = name;
    realName.erase(std::remove(realName.begin(), realName.end(), '*'), realName.end());

    auto refCount = std::count(name.begin(), name.end(), '*');

    realName = std::regex_replace(realName, rIndex, "");

    std::deque<int> dimensions;
    for (auto index = std::sregex_iterator{name.begin(), name.end(), rIndex}; index != std::sregex_iterator{}; index++) {
        dimensions.push_front(index->str(1).empty() ? -1 : std::stoi(index->str(1)));
    }

    if (named_types_.find(realName) == named_types_.end()) {
        return nullptr;
    }

    auto type = named_types_.at(realName);

    for (auto i = 0; i < refCount; i++) {
        type = type->getRef();
    }

    for (auto dim : dimensions) {
        type = type->getArray(dim);
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
    named_types_.insert(
        {"int", std::make_shared<BaseType>("int", [](auto& ctx) { return llvm::Type::getInt32Ty(ctx); })});
    named_types_.insert(
        {"void", std::make_shared<BaseType>("void", [](auto& ctx) { return llvm::Type::getVoidTy(ctx); })});
}

bool TypeLibrary::add(const std::string& name, const Type::Id& tid)
{
    if (named_types_.find(name) != named_types_.end()) {
        return false;
    }
    named_types_.insert({name, tid});
    return true;
}
