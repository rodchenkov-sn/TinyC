#include "TerminalWriter.h"

#include <llvm/IR/Module.h>

bool TerminalWriter::consume(std::any data)
{
    if (data.type() != typeid(llvm::Module*)) {
        return false;
    }
    auto* module = std::any_cast<llvm::Module*>(data);
    module->print(llvm::outs(), nullptr);
    delete module;
    return true;
}
