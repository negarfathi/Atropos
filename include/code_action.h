#ifndef CODE_ACTION_H
#define CODE_ACTION_H

#include "clang/Frontend/FrontendAction.h"

#include "code_consumer.h"

using namespace clang;

class CodeAction : public clang::ASTFrontendAction {
public:
    explicit CodeAction(int& loopCount) : loopCount(loopCount) {}

    virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &Compiler, llvm::StringRef InFile);

private:
    clang::Rewriter TheRewriter;
    int& loopCount;
};

#endif //CODE_ACTION_H