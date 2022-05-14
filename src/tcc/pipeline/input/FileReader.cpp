#include "FileReader.h"

#include "TinyCLexer.h"
#include "TinyCParser.h"

FileReader::FileReader(std::string fileName)
    : file_name_(std::move(fileName))
{
}

std::any FileReader::produce()
{
    std::ifstream file;
    file.open(file_name_);
    if (!file.is_open()) {
        return {};
    }

    input_stream_ = std::make_unique<antlr4::ANTLRInputStream>(file);
    lexer_ = std::make_unique<TinyCLexer>(input_stream_.get());
    tokens_ = std::make_unique<antlr4::CommonTokenStream>(lexer_.get());
    parser_ = std::make_unique<TinyCParser>(tokens_.get());

    if (parser_->getNumberOfSyntaxErrors() != 0) {
        return {};
    }
    return parser_->translationUnit();
}
