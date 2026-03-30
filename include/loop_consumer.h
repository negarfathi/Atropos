#ifndef LOOP_CONSUMER_H
#define LOOP_CONSUMER_H

#include "clang/Frontend/CompilerInstance.h"

#include "loop_visitor.h"

using namespace clang;

class LoopConsumer : public clang::ASTConsumer {
public:
    explicit LoopConsumer(ASTContext *Context, Rewriter &R) : Visitor(Context, R), TheRewriter(R) {}

    virtual void HandleTranslationUnit(clang::ASTContext &Context);

private:
    LoopVisitor Visitor;
    clang::Rewriter &TheRewriter;
};

#endif //LOOP_CONSUMER_H