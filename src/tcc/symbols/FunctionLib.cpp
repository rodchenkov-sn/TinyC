#include "FunctionLib.h"

static FunctionLibrary* instance = nullptr;

FunctionLibrary& FunctionLibrary::inst()
{
    if (!instance) {
        instance = new FunctionLibrary;
    }
    return *instance;
}

FunctionId FunctionLibrary::get(const std::string& name)
{
    if (named_functions_.find(name) != named_functions_.end()) {
        return &named_functions_.at(name);
    }
    return nullptr;
}

FunctionId FunctionLibrary::add(const Function& function)
{
    if (named_functions_.find(function.name) != named_functions_.end()) {
        return nullptr;
    }
    named_functions_.insert({function.name, function});
    return get(function.name);
}
