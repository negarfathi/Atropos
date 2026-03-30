#include "../include/loop_action.h"

std::unique_ptr<clang::ASTConsumer> LoopAction::CreateASTConsumer(clang::CompilerInstance &Compiler, llvm::StringRef InFile) {
    TheRewriter.setSourceMgr(Compiler.getSourceManager(), Compiler.getLangOpts());
    return std::make_unique<LoopConsumer>(&Compiler.getASTContext(), TheRewriter);
}