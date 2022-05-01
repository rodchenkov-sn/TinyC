#include "FunctionLib.h"


static FunctionLibrary* instance = nullptr;


FunctionLibrary& FunctionLibrary::inst()
{
    if (!instance) {
        instance = new FunctionLibrary;
    }
    return *instance;
}


const Function* FunctionLibrary::get(const std::string& name) const
{
    if (named_functions_.find(name) != named_functions_.end()) {
        return &named_functions_.at(name);
    }
    return nullptr;
}


bool FunctionLibrary::add(const Function& function)
{
    if (named_functions_.find(function.name) != named_functions_.end()) {
        return false;
    }
    named_functions_.insert({ function.name, function });
    return true;
}
