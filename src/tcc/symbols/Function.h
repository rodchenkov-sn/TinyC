#ifndef TINYC_FUNCTION_H
#define TINYC_FUNCTION_H


#include <string>
#include <vector>

#include "Type.h"


struct Function {
    std::string name;
    Type::Id returnType;
    std::vector<Type::Id> parameters;
};


using FunctionId = const Function*;


#endif //TINYC_FUNCTION_H
