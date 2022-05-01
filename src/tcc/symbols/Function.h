#ifndef TINYC_FUNCTION_H
#define TINYC_FUNCTION_H


#include <string>
#include <vector>

#include "Type.h"


struct Function {
    std::string name;
    const Type* returnType;
    std::vector<const Type*> parameters;
};


using FunctionId = const Function*;


#endif //TINYC_FUNCTION_H
