#include "../include/variable_action.h"

std::unique_ptr<clang::ASTConsumer> VariableAction::CreateASTConsumer(clang::CompilerInstance &Compiler, llvm::StringRef InFile) {
    TheRewriter.setSourceMgr(Compiler.getSourceManager(), Compiler.getLangOpts());
    return std::make_unique<VariableConsumer>(&Compiler.getASTContext(), TheRewriter, functionName, lineNumber, llFile, t2File);
}