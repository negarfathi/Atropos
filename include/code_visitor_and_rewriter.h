#ifndef CODE_VISITOR_AND_REWRITER_H
#define CODE_VISITOR_AND_REWRITER_H

#include <fstream>
#include <sstream>

#include "clang/Lex/Lexer.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/AST/RecursiveASTVisitor.h"

class CodeVisitorAndRewriter : public clang::RecursiveASTVisitor<CodeVisitorAndRewriter> {
    int counter = 0;
    std::string textToInsert1 = "";
    bool isInsideIfCondition;
    bool hasNondetCall;
    bool isFirstFunction = true;
    clang::SourceLocation mainFunctionStartLoc;
    std::string currentFunctionName;

public:
    explicit CodeVisitorAndRewriter(clang::ASTContext *Context, clang::Rewriter &R, int& loopCountRef) : Context(Context), TheRewriter(R), loopCount(loopCountRef) {}

    bool VisitFunctionDecl(clang::FunctionDecl *FD);
    bool VisitForStmt(clang::ForStmt *FS);
    bool VisitWhileStmt(clang::WhileStmt *WS);
    bool VisitDoStmt(clang::DoStmt *DS);
    bool VisitVarDecl(clang::VarDecl *VD);
    bool VisitBinaryOperator(clang::BinaryOperator *BO);
    bool VisitIfStmt(clang::IfStmt *IS);
    bool VisitCallExpr(clang::CallExpr *CE);
    bool VisitReturnStmt(clang::ReturnStmt *RS);
    bool VisitDeclRefExpr(clang::DeclRefExpr *DRE);

    std::string GenerateProgramState(std::string txtFilePath, int counter);

private:
    clang::ASTContext *Context;
    clang::Rewriter &TheRewriter;
    int& loopCount;
};

#endif //CODE_VISITOR_AND_REWRITER_H