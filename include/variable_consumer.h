#ifndef VARIABLE_CONSUMER_H
#define VARIABLE_CONSUMER_H

#include "clang/Frontend/CompilerInstance.h"

#include "variable_visitor.h"

using namespace clang;

class VariableConsumer : public clang::ASTConsumer {
public:
    explicit VariableConsumer(ASTContext *Context, Rewriter &R, std::string& functionName, std::string& lineNumber, std::string& llFile, std::string& t2File) : Visitor(Context, R, functionName, lineNumber, llFile, t2File), TheRewriter(R) {}

    virtual void HandleTranslationUnit(clang::ASTContext &Context);

private:
    VariableVisitor Visitor;
    clang::Rewriter &TheRewriter;
};

#endif //VARIABLE_CONSUMER_H