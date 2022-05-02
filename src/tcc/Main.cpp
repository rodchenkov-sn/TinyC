#include <iostream>

#include "TinyCLexer.h"
#include "TinyCParser.h"

#include "ast/AstVisitor.h"
#include "asg/AsgNode.h"
#include "symbols/SymbolResolver.h"
#include "ir/IrEmitter.h"


int main()
{
    std::ifstream file;
    file.open("../examples/flow_control/FlowControl.c");

    if (!file.is_open()) {
        std::cout << "could not open input";
        return 1;
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
        return 1;
    }

    IrEmitter emitter;
    auto module = emitter.emmit(root, "Bruh");

    module->print(llvm::errs(), nullptr);

    return 0;
}
