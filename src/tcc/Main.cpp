#include <iostream>

#include "argparse/argparse.hpp"

#include "TinyCLexer.h"
#include "TinyCParser.h"

#include "ast/AstVisitor.h"
#include "asg/AsgNode.h"
#include "symbols/SymbolResolver.h"
#include "ir/IrEmitter.h"


int main(int argc, char** argv)
{
    argparse::ArgumentParser program{ "tcc" };

    program.add_argument("input")
        .help("specify the input file")
        .required()
        ;

    program.add_argument("-o", "--output")
        .help("specify the output file")
        .default_value(std::string{})
        ;

    program.add_argument("--no-opt")
        .help("disable optimizations")
        .default_value(false)
        .implicit_value(true)
        ;

    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return EXIT_FAILURE;
    }

    std::ifstream file;
    auto inputFileName = program.get<std::string>("input");
    file.open(inputFileName);

    if (!file.is_open()) {
        std::cerr << "could not open file " << program.get<std::string>("input");
        return EXIT_FAILURE;
    }

    antlr4::ANTLRInputStream inputStream{ file };
    TinyCLexer lexer{ &inputStream };
    antlr4::CommonTokenStream tokens{ &lexer };
    TinyCParser parser{ &tokens };
    TinyCParser::TranslationUnitContext* context = parser.translationUnit();

    AstVisitor visitor;
    auto* root = std::any_cast<AsgNode*>(visitor.visitTranslationUnit(context));

    SymbolResolver resolver;
    if (!resolver.resolve(root)) {
        for (auto& errorStream : resolver.getErrors()) {
            std::cerr << errorStream.str() << "\n";
        }
        return EXIT_FAILURE;
    }

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

    IrEmitter emitter;
    auto module = emitter.emit(
        root,
        moduleName,
        !program.get<bool>("--no-opt")
    );

    auto outputName = program.get<std::string>("-o");
    if (outputName.empty()) {
        outputName = inputFileName.erase(
            inputFileName.find_last_of('.'),
            inputFileName.size()
        ) + ".ll";
    }

    std::error_code ec;
    llvm::raw_fd_ostream ostream{ outputName, ec };

    if (ec) {
        std::cerr << ec.message();
        return EXIT_FAILURE;
    }

    module->print(ostream, nullptr);

    ostream.close();

    return EXIT_SUCCESS;
}
