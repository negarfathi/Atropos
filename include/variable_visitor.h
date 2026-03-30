#ifndef VARIABLE_VISITOR_H
#define VARIABLE_VISITOR_H

#include <regex>
#include <fstream>

#include "clang/Lex/Lexer.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/AST/RecursiveASTVisitor.h"

struct Structure {
    std::string cName;
    std::string t2Name;
    std::string field;
    int index;
    std::string fieldType;
};

struct OneDimArray {
    std::string cName;
    std::string t2Name;
    std::string size;
    std::string elementType;
};

struct TwoDimArray {
    std::string cName;
    std::string t2Name;
    std::string rowSize;
    std::string columnSize;
    std::string elementType;
};

struct Variable {
    std::string cName;
    std::string t2Name;
    std::string variableType;
};

class VariableVisitor : public clang::RecursiveASTVisitor<VariableVisitor> {
    bool flag = false;
    std::vector<Structure> structures;
    std::vector<OneDimArray> oneDimArrays;
    std::vector<TwoDimArray> twoDimArrays;
    std::vector<Variable> variables;

public:
    explicit VariableVisitor(clang::ASTContext *Context, clang::Rewriter &R, std::string& functionNameRef, std::string& lineNumberRef, std::string& llFileRef, std::string& t2FileRef) : Context(Context), TheRewriter(R), functionName(functionNameRef), lineNumber(lineNumberRef), llFile(llFileRef), t2File(t2FileRef) {}

    bool VisitFunctionDecl(clang::FunctionDecl *FD);
    bool VisitTypedefDecl(clang::TypedefDecl *TD);
    bool VisitForStmt(clang::ForStmt *FS);
    bool VisitWhileStmt(clang::WhileStmt *WS);
    bool VisitDoStmt(clang::DoStmt *DS);
    bool VisitVarDecl(clang::VarDecl *VD);

    std::string MapName(std::string category, std::string cName, unsigned declLine);
private:
    clang::ASTContext *Context;
    clang::Rewriter &TheRewriter;
    std::string& functionName;
    std::string& lineNumber;
    std::string& llFile;
    std::string& t2File;
};

#endif //VARIABLE_VISITOR_H