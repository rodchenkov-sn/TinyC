#include "TerminalWriter.h"

bool TerminalWriter::consume(std::any data)
{
    if (data.type() != typeid(llvm::Module*)) {
        TC_LOG_CRITICAL("Unexpected data type passed to TerminalWriter -- expected llvm::Module*");
        return false;
    }
    auto* module = std::any_cast<llvm::Module*>(data);
    module->print(llvm::outs(), nullptr);
    delete module;
    return true;
}
