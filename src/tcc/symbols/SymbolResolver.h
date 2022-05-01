#ifndef TINYC_SYMBOLRESOLVER_H
#define TINYC_SYMBOLRESOLVER_H


#include <unordered_map>
#include <string>
#include <stack>
#include <vector>
#include <sstream>

#include "asg/AsgVisitor.h"
#include "asg/AsgNode.h"

#include "Type.h"


class SymbolResolver : private AsgVisitorBase {
public:
    bool resolve(AsgNode* root);
    const std::vector<std::stringstream>& getErrors() const { return errors_; }

private:
    void visitStatementList(struct AsgStatementList* node) override;
    void visitFunctionDefinition(struct AsgFunctionDefinition* node) override;
    void visitVariableDefinition(struct AsgVariableDefinition* node) override;
    void visitReturn(struct AsgReturn* node) override;
    void visitAssignment(struct AsgAssignment* node) override;
    void visitAddSub(struct AsgAddSub* node) override;
    void visitMulDiv(struct AsgMulDiv* node) override;
    void visitVariable(struct AsgVariable* node) override;
    void visitCall(struct AsgCall* node) override;
    void visitIntLiteral(struct AsgIntLiteral* node) override;

    std::stack<std::unordered_map<std::string, const Type*>> local_variables_;
    std::vector<std::stringstream> errors_;
};


#endif //TINYC_SYMBOLRESOLVER_H
