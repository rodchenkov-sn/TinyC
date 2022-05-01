#ifndef TINYC_SYMBOLRESOLVER_H
#define TINYC_SYMBOLRESOLVER_H


#include <unordered_map>
#include <string>
#include <stack>
#include <vector>
#include <sstream>
#include <any>

#include "asg/AsgVisitor.h"
#include "asg/AsgNode.h"

#include "Type.h"


class SymbolResolver : private AsgVisitorBase {
public:
    bool resolve(AsgNode* root);
    const std::vector<std::stringstream>& getErrors() const { return errors_; }

private:
    std::any visitStatementList(struct AsgStatementList* node) override;
    std::any visitFunctionDefinition(struct AsgFunctionDefinition* node) override;
    std::any visitVariableDefinition(struct AsgVariableDefinition* node) override;
    std::any visitReturn(struct AsgReturn* node) override;
    std::any visitAssignment(struct AsgAssignment* node) override;
    std::any visitAddSub(struct AsgAddSub* node) override;
    std::any visitMulDiv(struct AsgMulDiv* node) override;
    std::any visitVariable(struct AsgVariable* node) override;
    std::any visitCall(struct AsgCall* node) override;
    std::any visitIntLiteral(struct AsgIntLiteral* node) override;

    TypeId findVarType(const std::string& name) const;

    std::vector<std::stringstream> errors_;

    AsgStatementList* top_scope_ = nullptr;
    AsgFunctionDefinition* current_function_ = nullptr;
};


#endif //TINYC_SYMBOLRESOLVER_H
