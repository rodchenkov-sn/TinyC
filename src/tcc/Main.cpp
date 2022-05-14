#include <iostream>

#include "argparse/argparse.hpp"
#include "ast/AstVisitor.h"
#include "ir/IrEmitter.h"
#include "pipeline/input/FileReader.h"
#include "pipeline/output/FileWriter.h"
#include "pipeline/output/TerminalWriter.h"
#include "pipeline/Pipeline.h"
#include "symbols/SymbolResolver.h"
#include "symbols/TypeResolver.h"

int main(int argc, char** argv)
{
    argparse::ArgumentParser program{"tcc"};

    program.add_argument("input")
        .help("specify the input file")
        .required();

    program.add_argument("-o", "--output")
        .help("specify the output file")
        .default_value(std::string{});

    program.add_argument("--no-opt")
        .help("disable optimizations")
        .default_value(false)
        .implicit_value(true);

    program.add_argument("-p", "--print")
        .help("print IR to stdout instead of creating output file")
        .default_value(false)
        .implicit_value(true);

    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return EXIT_FAILURE;
    }

    auto inputFileName = program.get<std::string>("input");

    auto moduleName = inputFileName;
    auto lastPathChar = moduleName.find_last_of('/');
    if (lastPathChar == std::string::npos) {
        lastPathChar = moduleName.find_last_of('\\');
    } else {
        auto l = moduleName.find_last_of('\\');
        if (l != std::string::npos && l > lastPathChar) {
            lastPathChar = l;
        }
    }
    if (lastPathChar != std::string::npos) {
        moduleName.erase(0, lastPathChar + 1);
    }

    Pipeline pipeline;

    pipeline
        .add(std::make_unique<FileReader>(program.get<std::string>("input")))
        .add(std::make_unique<AstVisitor>())
        .add(std::make_unique<SymbolResolver>())
        .add(std::make_unique<TypeResolver>())
        .add(std::make_unique<IrEmitter>(moduleName, !program.get<bool>("--no-opt")));

    if (program.get<bool>("-p")) {
        pipeline.add(std::make_unique<TerminalWriter>());
    } else {
        auto outputName = program.get<std::string>("-o");
        if (outputName.empty()) {
            outputName = inputFileName.erase(
                             inputFileName.find_last_of('.'),
                             inputFileName.size())
                       + ".ll";
        }
        pipeline.add(std::make_unique<FileWriter>(outputName));
    }

    return pipeline.run() ? EXIT_SUCCESS : EXIT_FAILURE;
}
