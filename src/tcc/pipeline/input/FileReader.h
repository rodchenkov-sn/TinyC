#ifndef TINYC_ASTGENERATOR_H
#define TINYC_ASTGENERATOR_H

#include <string>

#include "pipeline/PipelineStage.h"
#include "TinyCLexer.h"
#include "TinyCParser.h"

class FileReader : public PipeInputBase {
public:
    explicit FileReader(std::string fileName);

    std::any produce() override;

private:
    std::string file_name_;

    std::unique_ptr<antlr4::ANTLRInputStream> input_stream_;
    std::unique_ptr<TinyCLexer> lexer_;
    std::unique_ptr<antlr4::CommonTokenStream> tokens_;
    std::unique_ptr<TinyCParser> parser_;
};

#endif
