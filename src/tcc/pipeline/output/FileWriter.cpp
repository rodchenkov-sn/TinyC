#include "FileWriter.h"

#include <llvm/IR/Module.h>

FileWriter::FileWriter(std::string fileName)
    : file_name_(std::move(fileName))
{
}

bool FileWriter::consume(std::any data)
{
    if (data.type() != typeid(llvm::Module*)) {
        return false;
    }
    auto* module = std::any_cast<llvm::Module*>(data);

    std::error_code ec;
    llvm::raw_fd_ostream ostream{file_name_, ec};

    if (ec) {
        return false;
    }

    module->print(ostream, nullptr);

    ostream.close();

    delete module;
    return true;
}
