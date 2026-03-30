#ifndef VARIABLE_ACTION_H
#define VARIABLE_ACTION_H

#include "clang/Frontend/FrontendAction.h"

#include "variable_consumer.h"

using namespace clang;

class VariableAction : public clang::ASTFrontendAction {
public:
    explicit VariableAction(std::string& functionName, std::string& lineNumber, std::string& llFile, std::string& t2File) : functionName(functionName), lineNumber(lineNumber), llFile(llFile), t2File(t2File) {}

    virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &Compiler, llvm::StringRef InFile);

private:
    clang::Rewriter TheRewriter;
    std::string& functionName;
    std::string& lineNumber;
    std::string& llFile;
    std::string& t2File;
};

#endif //VARIABLE_ACTION_H