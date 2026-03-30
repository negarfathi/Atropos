#include "../include/loop_visitor.h"

bool LoopVisitor::VisitFunctionDecl(clang::FunctionDecl *FD) {
    if (FD->hasBody()) {
        clang::SourceManager &SM = TheRewriter.getSourceMgr();
        clang::SourceLocation functionSignatureStartLoc = FD->getBeginLoc();
        clang::SourceLocation functionSignatureEndLoc = FD->getBody()->getBeginLoc();
        clang::CharSourceRange range = clang::CharSourceRange::getTokenRange(functionSignatureStartLoc, functionSignatureEndLoc);
        currentFunctionSignature = clang::Lexer::getSourceText(range, SM, Context->getLangOpts()).str();

        currentFunctionName = FD->getNameAsString();
        currentFunctionReturnType = FD->getReturnType().getAsString();
    }
    else{
        clang::SourceManager &SM = TheRewriter.getSourceMgr();
        if (SM.isInMainFile(FD->getBeginLoc())) {
            clang::SourceLocation externalFunctionStartLoc = FD->getSourceRange().getBegin();
            clang::SourceLocation externalFunctionEndLoc = clang::Lexer::getLocForEndOfToken(FD->getSourceRange().getEnd(), 0, SM, Context->getLangOpts());
            clang::CharSourceRange range = clang::CharSourceRange::getCharRange(externalFunctionStartLoc, externalFunctionEndLoc);
            externalFunctions.push_back(clang::Lexer::getSourceText(range, SM, Context->getLangOpts()).str() + ";");
        }
    }
    return true;
}

bool LoopVisitor::VisitTypedefDecl(clang::TypedefDecl *TD) {
    clang::SourceManager &SM = TheRewriter.getSourceMgr();
    if (SM.isInMainFile(TD->getBeginLoc())) {
        clang::SourceLocation structureStartLoc = TD->getBeginLoc();
        clang::SourceLocation structureEndLoc = clang::Lexer::getLocForEndOfToken(TD->getEndLoc(), 0, SM, Context->getLangOpts());
        clang::CharSourceRange range = clang::CharSourceRange::getTokenRange(structureStartLoc, structureEndLoc);
        structures.push_back(clang::Lexer::getSourceText(range, SM, Context->getLangOpts()).str());
    }
    return true;
}

bool LoopVisitor::VisitVarDecl(clang::VarDecl *VD) {
    if (VD->hasGlobalStorage() && !VD->isStaticLocal()) {
        clang::SourceManager &SM = TheRewriter.getSourceMgr();
        if (SM.isInMainFile(VD->getBeginLoc())) {
            clang::SourceLocation varDeclStartLoc = VD->getBeginLoc();
            clang::SourceLocation varDeclEndLoc = clang::Lexer::getLocForEndOfToken(VD->getEndLoc(), 0, SM, Context->getLangOpts());
            clang::CharSourceRange range = clang::CharSourceRange::getCharRange(varDeclStartLoc, varDeclEndLoc);
            globalVariables.push_back(clang::Lexer::getSourceText(range, SM, Context->getLangOpts()).str() + ";");
        }
    }
    return true;
}

bool LoopVisitor::VisitForStmt(clang::ForStmt *FS) {
    if(!isInsideLoop){
        clang::SourceManager &SM = TheRewriter.getSourceMgr();
        loopStartLoc = FS->getForLoc();
        loopEndLoc = clang::Lexer::getLocForEndOfToken(FS->getEndLoc(), 0, SM, Context->getLangOpts());
        clang::CharSourceRange range = clang::CharSourceRange::getTokenRange(loopStartLoc, loopEndLoc);
        currentLoop = clang::Lexer::getSourceText(range, SM, Context->getLangOpts()).str();

        isInsideLoop = true;
        this->TraverseStmt(FS->getInit());
        this->TraverseStmt(FS->getCond());
        this->TraverseStmt(FS->getInc());
        this->TraverseStmt(FS->getBody());
        isInsideLoop = false;

        std::ofstream file("./Artifacts/Loop_" + currentFunctionName + "_" + std::to_string(SM.getPresumedLoc(loopStartLoc).getLine()) + ".c");
        if (file.is_open()) {
            for (const auto& externalFunc : externalFunctions) {
                file << externalFunc << "\n";
            }
            for (const auto& structure : structures) {
                file << structure << "\n";
            }
            for (const auto& globalVar : globalVariables) {
                file << globalVar << "\n";
            }
            for (const auto& funcCallDef : functionCallDefinitions) {
                file << funcCallDef << "\n";
            }
            file << currentFunctionSignature << "\n";
            for (const auto& missingLocalVar : missingLocalVariables) {
                file << missingLocalVar << "\n";
            }
            file << currentLoop << "\n";
            if(currentFunctionReturnType == "int"){
                file << "return 0;\n}\n";
            }
            else if(currentFunctionReturnType == "unsigned int"){
                file << "return 0;\n}\n";
            }
            file.close();
        }

        missingLocalVariables.clear();
        functionCallDefinitions.clear();
    }
    return true;
}

bool LoopVisitor::VisitWhileStmt(clang::WhileStmt *WS) {
    if(!isInsideLoop){
        clang::SourceManager &SM = TheRewriter.getSourceMgr();
        loopStartLoc = WS->getWhileLoc();
        loopEndLoc = clang::Lexer::getLocForEndOfToken(WS->getEndLoc(), 0, SM, Context->getLangOpts());
        clang::CharSourceRange range = clang::CharSourceRange::getTokenRange(loopStartLoc, loopEndLoc);
        currentLoop = clang::Lexer::getSourceText(range, SM, Context->getLangOpts()).str();

        isInsideLoop = true;
        this->TraverseStmt(WS->getCond());
        this->TraverseStmt(WS->getBody());
        isInsideLoop = false;

        std::ofstream file("./Artifacts/Loop_" + currentFunctionName + "_" + std::to_string(SM.getPresumedLoc(loopStartLoc).getLine()) + ".c");
        if (file.is_open()) {
            for (const auto &externalFunc: externalFunctions) {
                file << externalFunc << "\n";
            }
            for (const auto &structure: structures) {
                file << structure << "\n";
            }
            for (const auto &globalVar: globalVariables) {
                file << globalVar << "\n";
            }
            for (const auto &funcCallDef: functionCallDefinitions) {
                file << funcCallDef << "\n";
            }
            file << currentFunctionSignature << "\n";
            for (const auto &missingLocalVar: missingLocalVariables) {
                file << missingLocalVar << "\n";
            }
            file << currentLoop << "\n";
            if (currentFunctionReturnType == "int") {
                file << "return 0;\n}\n";
            }
            else if(currentFunctionReturnType == "unsigned int"){
                file << "return 0;\n}\n";
            }
            file.close();
        }

        missingLocalVariables.clear();
        functionCallDefinitions.clear();
    }
    return true;
}

bool LoopVisitor::VisitDoStmt(clang::DoStmt *DS) {
    if(!isInsideLoop){
        clang::SourceManager &SM = TheRewriter.getSourceMgr();
        loopStartLoc = DS->getDoLoc();
        loopEndLoc = clang::Lexer::getLocForEndOfToken(DS->getEndLoc(), 0, SM, Context->getLangOpts());
        clang::CharSourceRange range = clang::CharSourceRange::getTokenRange(loopStartLoc, loopEndLoc);
        currentLoop = clang::Lexer::getSourceText(range, SM, Context->getLangOpts()).str();

        isInsideLoop = true;
        this->TraverseStmt(DS->getCond());
        this->TraverseStmt(DS->getBody());
        isInsideLoop = false;

        std::ofstream file("./Artifacts/Loop_" + currentFunctionName + "_" + std::to_string(SM.getPresumedLoc(loopStartLoc).getLine()) + ".c");
        if (file.is_open()) {
            for (const auto& externalFunc : externalFunctions) {
                file << externalFunc << "\n";
            }
            for (const auto& structure : structures) {
                file << structure << "\n";
            }
            for (const auto& globalVar : globalVariables) {
                file << globalVar << "\n";
            }
            for (const auto& funcCallDef : functionCallDefinitions) {
                file << funcCallDef << "\n";
            }
            file << currentFunctionSignature << "\n";
            for (const auto& missingLocalVar : missingLocalVariables) {
                file << missingLocalVar << "\n";
            }
            file << currentLoop << "\n";
            if(currentFunctionReturnType == "int"){
                file << "return 0;\n}\n";
            }
            else if(currentFunctionReturnType == "unsigned int"){
                file << "return 0;\n}\n";
            }
            file.close();
        }

        missingLocalVariables.clear();
        functionCallDefinitions.clear();
    }
    return true;
}

bool LoopVisitor::VisitDeclRefExpr(clang::DeclRefExpr *DRE) {
    if (isInsideLoop) {
        clang::Decl* decl = DRE->getDecl();

        if (llvm::isa<clang::VarDecl>(decl)) {
            clang::VarDecl* varDecl = llvm::cast<clang::VarDecl>(decl);

            if (varDecl->isLocalVarDecl()) {
                clang::QualType varType = varDecl->getType();
                std::string varName = varDecl->getNameAsString();
                std::string varInit;
                std::string externalFunctionStr;

                clang::SourceManager &SM = TheRewriter.getSourceMgr();
                clang::SourceLocation varDeclStartLoc = varDecl->getBeginLoc();

                if (!(varDeclStartLoc > loopStartLoc && varDeclStartLoc <= loopEndLoc)) {
                    if (const clang::BuiltinType* bt = varType->getAs<clang::BuiltinType>()) {
                        switch (bt->getKind()) {
                            case clang::BuiltinType::Float:
                                varInit = "__VERIFIER_nondet_float()";
                                externalFunctionStr = "extern float __VERIFIER_nondet_float(void);";
                                break;
                            case clang::BuiltinType::Long:
                                varInit = "__VERIFIER_nondet_long()";
                                externalFunctionStr = "extern long __VERIFIER_nondet_long(void);";
                                break;
                            case clang::BuiltinType::Int:
                                varInit = "__VERIFIER_nondet_int()";
                                externalFunctionStr = "extern int __VERIFIER_nondet_int(void);";
                                break;
                            case clang::BuiltinType::Char_S:
                            case clang::BuiltinType::Char_U:
                                varInit = "__VERIFIER_nondet_char()";
                                externalFunctionStr = "extern char __VERIFIER_nondet_char(void);";
                                break;
                            case clang::BuiltinType::ULong:
                                varInit = "__VERIFIER_nondet_ulong()";
                                externalFunctionStr = "extern unsigned long __VERIFIER_nondet_ulong(void);";
                                break;
                            case clang::BuiltinType::UInt:
                                varInit = "__VERIFIER_nondet_uint()";
                                externalFunctionStr = "extern unsigned int __VERIFIER_nondet_uint(void);";
                                break;
                            case clang::BuiltinType::UShort:
                                varInit = "__VERIFIER_nondet_ushort()";
                                externalFunctionStr = "extern unsigned short __VERIFIER_nondet_ushort(void);";
                                break;
                            case clang::BuiltinType::UChar:
                                varInit = "__VERIFIER_nondet_uchar()";
                                externalFunctionStr = "extern unsigned char __VERIFIER_nondet_uchar(void);";
                                break;
                        }
                        std::string varDeclStr = varType.getAsString() + " " + varName + " = " + varInit + ";";
                        if (std::find(missingLocalVariables.begin(), missingLocalVariables.end(), varDeclStr) == missingLocalVariables.end()) {
                            missingLocalVariables.push_back(varDeclStr);

                            if (std::find(externalFunctions.begin(), externalFunctions.end(), externalFunctionStr) == externalFunctions.end()) {
                                externalFunctions.push_back(externalFunctionStr);
                            }
                        }
                    }

                    else if (varType->isArrayType()) {
                        clang::SourceLocation varDeclEndLoc = clang::Lexer::getLocForEndOfToken(varDecl->getEndLoc(), 0, SM, Context->getLangOpts());
                        clang::CharSourceRange range = clang::CharSourceRange::getCharRange(varDeclStartLoc, varDeclEndLoc);
                        std::string varDeclStr = clang::Lexer::getSourceText(range, SM, Context->getLangOpts()).str() + ";";

                        if (varDeclStr.find(',') != std::string::npos) {
                            std::regex typePattern("(\\w+)\\[([0-9]+)\\]");
                            std::smatch matches;
                            std::string varTypeStr = varType.getAsString();
                            if (std::regex_match(varTypeStr, matches, typePattern)) {
                                varDeclStr = matches[1].str() + " " + varName + "[" + matches[2].str() + "];";
                            }
                        }

                        std::string arrayDecl = varDeclStr;
                        std::regex dimensionRegex("\\[(.*?)\\]");
                        std::smatch matches;
                        while (std::regex_search(arrayDecl, matches, dimensionRegex)) {
                            std::string dimension = matches[1];
                            std::regex constantRegex("^[0-9]+$");
                            if (!(std::regex_match(dimension, constantRegex))) {
                                std::string dimensionDecl = "int " + dimension + " = __VERIFIER_nondet_int();";
                                if (std::find(missingLocalVariables.begin(), missingLocalVariables.end(), dimensionDecl) == missingLocalVariables.end()) {
                                    missingLocalVariables.push_back(dimensionDecl);
                                }
                            }
                            arrayDecl = matches.suffix().str();
                        }

                        if (std::find(missingLocalVariables.begin(), missingLocalVariables.end(), varDeclStr) == missingLocalVariables.end()) {
                            missingLocalVariables.push_back(varDeclStr);
                        }
                    }
                }
            }
        }
    }
    return true;
}

bool LoopVisitor::VisitCallExpr(clang::CallExpr *CE) {
    if (isInsideLoop) {
        clang::FunctionDecl *funcDecl = CE->getDirectCallee();
        if (funcDecl->hasBody()) {
            clang::SourceManager &SM = Context->getSourceManager();
            clang::SourceLocation funcDeclStartLoc = funcDecl->getSourceRange().getBegin();
            clang::SourceLocation funcDeclEndLoc = clang::Lexer::getLocForEndOfToken(funcDecl->getBody()->getEndLoc(), 0, SM, Context->getLangOpts());
            clang::CharSourceRange Range = clang::CharSourceRange::getCharRange(funcDeclStartLoc, funcDeclEndLoc);
            std::string funcDeclStr = clang::Lexer::getSourceText(Range, SM, Context->getLangOpts()).str();
            if (std::find(functionCallDefinitions.begin(), functionCallDefinitions.end(), funcDeclStr) == functionCallDefinitions.end()) {
                functionCallDefinitions.push_back(funcDeclStr);
            }
        }
    }
    return true;
}