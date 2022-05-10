#ifndef TINYC_FUNCTIONLIB_H
#define TINYC_FUNCTIONLIB_H

#include <stdexcept>
#include <string>
#include <unordered_map>

#include "Function.h"

class FunctionLibrary {
public:
    static FunctionLibrary& inst();

    FunctionId get(const std::string& name) const;
    FunctionId add(const Function& function);

private:
    FunctionLibrary() = default;

    std::unordered_map<std::string, Function> named_functions_;
};

#endif
