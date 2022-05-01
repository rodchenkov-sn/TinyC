#ifndef TINYC_TCTYPE_H
#define TINYC_TCTYPE_H


#include <functional>

#include <llvm/IR/Type.h>


struct Type {
    std::function<llvm::Type*(llvm::LLVMContext&)> irTypeGetter;
};


using TypeId = const Type*;


#endif // TINYC_TCTYPE_H
