#ifndef TINYC_FUNCTIONLIB_H
#define TINYC_FUNCTIONLIB_H


#include <unordered_map>
#include <string>
#include <stdexcept>

#include "Function.h"


class FunctionLibrary {
public:
    static FunctionLibrary& inst();

    const Function* get(const std::string& name) const;
    bool add(const Function& function);

private:
    FunctionLibrary() = default;

    std::unordered_map<std::string, Function> named_functions_;
};


#endif //TINYC_FUNCTIONLIB_H
