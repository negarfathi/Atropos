#ifndef CODE_CONSUMER_H
#define CODE_CONSUMER_H

#include "clang/Frontend/CompilerInstance.h"

#include "code_visitor_and_rewriter.h"

using namespace clang;

class CodeConsumer : public clang::ASTConsumer {
public:
    explicit CodeConsumer(ASTContext *Context, Rewriter &R, int& loopCount) : Visitor(Context, R, loopCount), TheRewriter(R) {}

    virtual void HandleTranslationUnit(clang::ASTContext &Context);

private:
    CodeVisitorAndRewriter Visitor;
    clang::Rewriter &TheRewriter;
};

#endif //CODE_CONSUMER_H