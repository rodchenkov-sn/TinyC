#ifndef TINYC_ASTGENERATOR_H
#define TINYC_ASTGENERATOR_H

#include "pipeline/PipelineStage.h"
#include "TinyCLexer.h"
#include "TinyCParser.h"

class FileReader : public PipeInputBase {
public:
    explicit FileReader(std::string fileName);

    std::any produce() override;

private:
    struct ErrorListener : public antlr4::ANTLRErrorListener {
        void syntaxError(antlr4::Recognizer* recognizer, antlr4::Token* offendingSymbol, size_t line, size_t charPositionInLine, const std::string& msg, std::exception_ptr e) override;

        void reportAmbiguity(antlr4::Parser* recognizer, const antlr4::dfa::DFA& dfa, size_t startIndex, size_t stopIndex, bool exact, const antlrcpp::BitSet& ambigAlts, antlr4::atn::ATNConfigSet* configs) override
        {
        }

        void reportAttemptingFullContext(antlr4::Parser* recognizer, const antlr4::dfa::DFA& dfa, size_t startIndex, size_t stopIndex, const antlrcpp::BitSet& conflictingAlts, antlr4::atn::ATNConfigSet* configs) override
        {
        }

        void reportContextSensitivity(antlr4::Parser* recognizer, const antlr4::dfa::DFA& dfa, size_t startIndex, size_t stopIndex, size_t prediction, antlr4::atn::ATNConfigSet* configs) override
        {
        }

        bool hasErrors = false;
    };

    std::string file_name_;

    std::unique_ptr<antlr4::ANTLRInputStream> input_stream_;
    std::unique_ptr<TinyCLexer> lexer_;
    std::unique_ptr<antlr4::CommonTokenStream> tokens_;
    std::unique_ptr<TinyCParser> parser_;

    std::unique_ptr<ErrorListener> error_listener_;
};

#endif
