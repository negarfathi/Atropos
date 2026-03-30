#ifndef LOOP_VISITOR_H
#define LOOP_VISITOR_H

#include <regex>
#include <fstream>

#include "clang/Lex/Lexer.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/AST/RecursiveASTVisitor.h"

class LoopVisitor : public clang::RecursiveASTVisitor<LoopVisitor> {
    bool isInsideLoop = false;
    std::vector<std::string> externalFunctions;
    std::vector<std::string> structures;
    std::vector<std::string> globalVariables;
    std::vector<std::string> functionCallDefinitions;
    std::string currentFunctionName;
    std::string currentFunctionReturnType;
    std::string currentFunctionSignature;
    std::vector<std::string> missingLocalVariables;
    std::string currentLoop;
    clang::SourceLocation loopStartLoc;
    clang::SourceLocation loopEndLoc;

public:
    explicit LoopVisitor(clang::ASTContext *Context, clang::Rewriter &R) : Context(Context), TheRewriter(R) {}

    bool VisitFunctionDecl(clang::FunctionDecl *FD);
    bool VisitTypedefDecl(clang::TypedefDecl *TD);
    bool VisitVarDecl(clang::VarDecl *VD);
    bool VisitForStmt(clang::ForStmt *FS);
    bool VisitWhileStmt(clang::WhileStmt *WS);
    bool VisitDoStmt(clang::DoStmt *DS);
    bool VisitDeclRefExpr(clang::DeclRefExpr *DRE);
    bool VisitCallExpr(clang::CallExpr *CE);

private:
    clang::ASTContext *Context;
    clang::Rewriter &TheRewriter;
};

#endif //LOOP_VISITOR_H