#ifndef TINYC_FUNCTION_H
#define TINYC_FUNCTION_H

#include <string>
#include <vector>

#include "Type.h"

struct Function {
    std::string name;
    Type::Id returnType;
    Type::Id origRetType = Type::invalid();
    std::vector<Type::Id> parameters;
};

using FunctionId = Function*;

#endif
