#ifndef LOOP_ACTION_H
#define LOOP_ACTION_H

#include "clang/Frontend/FrontendAction.h"

#include "loop_consumer.h"

using namespace clang;

class LoopAction : public clang::ASTFrontendAction {
public:
    explicit LoopAction() {}

    virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &Compiler, llvm::StringRef InFile);

private:
    clang::Rewriter TheRewriter;
};

#endif //LOOP_ACTION_H