#include "FileWriter.h"

#include <llvm/IR/Module.h>
#include <spdlog/spdlog.h>

FileWriter::FileWriter(std::string fileName)
    : file_name_(std::move(fileName))
{
}

bool FileWriter::consume(std::any data)
{
    if (data.type() != typeid(llvm::Module*)) {
        spdlog::critical("Unexpected data type passed to FileWriter -- expected llvm::Module*");
        return false;
    }
    auto* module = std::any_cast<llvm::Module*>(data);

    std::error_code ec;
    llvm::raw_fd_ostream ostream{file_name_, ec};

    if (ec) {
        spdlog::error("can not open file {} for write -- {}", file_name_, ec.message());
        return false;
    }

    module->print(ostream, nullptr);

    ostream.close();

    delete module;
    return true;
}