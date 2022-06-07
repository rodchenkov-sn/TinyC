#include "FileReader.h"

#include "log/Logging.h"
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
        TC_LOG_CRITICAL("cant open file {}", file_name_);
        return {};
    }

    input_stream_ = std::make_unique<antlr4::ANTLRInputStream>(file);
    lexer_ = std::make_unique<TinyCLexer>(input_stream_.get());
    tokens_ = std::make_unique<antlr4::CommonTokenStream>(lexer_.get());
    parser_ = std::make_unique<TinyCParser>(tokens_.get());

    error_listener_ = std::make_unique<ErrorListener>();

    parser_->removeErrorListeners();
    parser_->addErrorListener(error_listener_.get());

    auto* ret = parser_->translationUnit();

    if (error_listener_->hasErrors) {
        return {};
    }

    return ret;
}

void FileReader::ErrorListener::syntaxError(antlr4::Recognizer* recognizer, antlr4::Token* offendingSymbol, size_t line, size_t charPositionInLine, const std::string& msg, std::exception_ptr e)
{
    TC_LOG_ERROR("at line {} -- {}", line, msg);
    hasErrors = true;
}
