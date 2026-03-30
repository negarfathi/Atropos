#include "../include/variable_visitor.h"

bool VariableVisitor::VisitFunctionDecl(clang::FunctionDecl *FD) {
    if(FD->getNameAsString() == functionName){
        flag = true;
    }
    return true;
}

bool VariableVisitor::VisitTypedefDecl(clang::TypedefDecl *TD) {
    clang::SourceManager &SM = TheRewriter.getSourceMgr();
    if (SM.isInMainFile(TD->getBeginLoc())) {
        clang::QualType QT = TD->getUnderlyingType();
        if (QT->isRecordType()) {
            const clang::RecordDecl *RD = QT->getAs<clang::RecordType>()->getDecl();
            if (RD->isStruct()) {
                std::string cName = RD->getNameAsString();
                std::string t2Name = "vstruct." + cName;
                int index = 0;
                for (const auto *field : RD->fields()) {
                    structures.push_back({cName, t2Name, field->getNameAsString(), index, field->getType().getAsString()});
                    index++;
                }
            }
        }
    }
    return true;
}

bool VariableVisitor::VisitForStmt(clang::ForStmt *FS) {
    flag = false;

    std::ofstream file("./Artifacts/LoopInfo_" + functionName + "_" + lineNumber + ".txt");
    if (file.is_open()) {
        for (const auto& item : variables) {
            if (t2File.find(item.t2Name) != std::string::npos) {
                file << "Variable," << item.cName << "," << item.t2Name << "," << item.variableType << "\n";
            }
        }
        for (const auto& item : oneDimArrays) {
            file << "OneDimArray," << item.cName << "," << item.t2Name << "," << item.size << "," << item.elementType << "\n";
        }
        for (const auto& item : twoDimArrays) {
            file << "TwoDimArray," << item.cName << "," << item.t2Name << "," << item.rowSize << "," << item.columnSize << "," << item.elementType << "\n";
        }
        for (const auto& item : structures) {
            file << "Structure," << item.cName << "," << item.t2Name << "," << item.field << "," << item.index << "," << item.fieldType << "\n";
        }
        file.close();
    }

    return true;
}

bool VariableVisitor::VisitWhileStmt(clang::WhileStmt *WS) {
    flag = false;

    std::ofstream file("./Artifacts/LoopInfo_" + functionName + "_" + lineNumber + ".txt");
    if (file.is_open()) {
        for (const auto& item : variables) {
            if (t2File.find(item.t2Name) != std::string::npos) {
                file << "Variable," << item.cName << "," << item.t2Name << "," << item.variableType << "\n";
            }
        }
        for (const auto& item : oneDimArrays) {
            file << "OneDimArray," << item.cName << "," << item.t2Name << "," << item.size << "," << item.elementType << "\n";
        }
        for (const auto& item : twoDimArrays) {
            file << "TwoDimArray," << item.cName << "," << item.t2Name << "," << item.rowSize << "," << item.columnSize << "," << item.elementType << "\n";
        }
        for (const auto& item : structures) {
            file << "Structure," << item.cName << "," << item.t2Name << "," << item.field << "," << item.index << "," << item.fieldType << "\n";
        }
        file.close();
    }

    return true;
}

bool VariableVisitor::VisitDoStmt(clang::DoStmt *DS) {
    flag = false;

    std::ofstream file("./Artifacts/LoopInfo_" + functionName + "_" + lineNumber + ".txt");
    if (file.is_open()) {
        for (const auto& item : variables) {
            if (t2File.find(item.t2Name) != std::string::npos) {
                file << "Variable," << item.cName << "," << item.t2Name << "," << item.variableType << "\n";
            }
        }
        for (const auto& item : oneDimArrays) {
            file << "OneDimArray," << item.cName << "," << item.t2Name << "," << item.size << "," << item.elementType << "\n";
        }
        for (const auto& item : twoDimArrays) {
            file << "TwoDimArray," << item.cName << "," << item.t2Name << "," << item.rowSize << "," << item.columnSize << "," << item.elementType << "\n";
        }
        for (const auto& item : structures) {
            file << "Structure," << item.cName << "," << item.t2Name << "," << item.field << "," << item.index << "," << item.fieldType << "\n";
        }
        file.close();
    }

    return true;
}

bool VariableVisitor::VisitVarDecl(clang::VarDecl *VD) {
    if (VD->hasGlobalStorage() && !VD->isStaticLocal()) {
        clang::SourceManager &SM = TheRewriter.getSourceMgr();
        if (SM.isInMainFile(VD->getBeginLoc())) {
            clang::QualType varType = VD->getType();
            if (varType->isArrayType()) {
                std::string cName = VD->getNameAsString();
                std::string t2Name = "v" + cName;
                std::string size;
                const clang::ArrayType *arrayType = clang::cast<clang::ArrayType>(varType.getTypePtr());
                if (const clang::ConstantArrayType *constArrayType = dyn_cast<clang::ConstantArrayType>(arrayType)) {
                    size = std::to_string(constArrayType->getSize().getZExtValue());
                }
                else if (const clang::VariableArrayType *varArrayType = dyn_cast<clang::VariableArrayType>(arrayType)) {
                    clang::Expr *sizeExpr = varArrayType->getSizeExpr();
                    clang::SourceLocation b(sizeExpr->getBeginLoc()), _e(sizeExpr->getEndLoc());
                    size = std::string(SM.getCharacterData(b), SM.getCharacterData(_e) - SM.getCharacterData(b));
                }
                oneDimArrays.push_back({cName, t2Name, size, arrayType->getElementType().getAsString()});
            }
        }
    }

    if (flag) {
        clang::SourceManager &SM = TheRewriter.getSourceMgr();
        unsigned declLine = SM.getSpellingLineNumber(VD->getLocation());

        clang::QualType varType = VD->getType();
        if (varType->isArrayType()) {
            const clang::ArrayType *arrayType = clang::cast<clang::ArrayType>(varType.getTypePtr());

            clang::QualType elementType = arrayType->getElementType();
            if (elementType->isArrayType()) {
                std::string cName = VD->getNameAsString();
                std::string t2Name = MapName("Array", cName, declLine);
                std::string rowSize;
                if (const clang::ConstantArrayType *constArrayType = dyn_cast<clang::ConstantArrayType>(arrayType)) {
                    rowSize = std::to_string(constArrayType->getSize().getZExtValue());
                }
                else if (const clang::VariableArrayType *varArrayType = dyn_cast<clang::VariableArrayType>(arrayType)) {
                    clang::Expr *rowSizeExpr = varArrayType->getSizeExpr();
                    clang::SourceLocation startLoc = rowSizeExpr->getBeginLoc();
                    clang::SourceLocation endLoc = clang::Lexer::getLocForEndOfToken(rowSizeExpr->getEndLoc(), 0, SM, Context->getLangOpts());
                    clang::CharSourceRange range = clang::CharSourceRange::getCharRange(startLoc, endLoc);
                    rowSize = clang::Lexer::getSourceText(range, SM, Context->getLangOpts()).str();
                }
                std::string columnSize;
                const clang::ArrayType *innerArrayType = clang::cast<clang::ArrayType>(elementType.getTypePtr());
                if (const clang::ConstantArrayType *constInnerArrayType = dyn_cast<clang::ConstantArrayType>(innerArrayType)) {
                    columnSize = std::to_string(constInnerArrayType->getSize().getZExtValue());
                }
                else if (const clang::VariableArrayType *varInnerArrayType = dyn_cast<clang::VariableArrayType>(innerArrayType)) {
                    clang::Expr *columnSizeExpr = varInnerArrayType->getSizeExpr();
                    clang::SourceLocation startLoc = columnSizeExpr->getBeginLoc();
                    clang::SourceLocation endLoc = clang::Lexer::getLocForEndOfToken(columnSizeExpr->getEndLoc(), 0, SM, Context->getLangOpts());
                    clang::CharSourceRange range = clang::CharSourceRange::getCharRange(startLoc, endLoc);
                    columnSize = clang::Lexer::getSourceText(range, SM, Context->getLangOpts()).str();
                }
                twoDimArrays.push_back({cName, t2Name, rowSize, columnSize, innerArrayType->getElementType().getAsString()});
            }

            else {
                std::string cName = VD->getNameAsString();
                std::string t2Name = MapName("Array", cName, declLine);
                std::string size;
                if (const clang::ConstantArrayType *constArrayType = dyn_cast<clang::ConstantArrayType>(arrayType)) {
                    size = std::to_string(constArrayType->getSize().getZExtValue());
                }
                else if (const clang::VariableArrayType *varArrayType = dyn_cast<clang::VariableArrayType>(arrayType)) {
                    clang::Expr *sizeExpr = varArrayType->getSizeExpr();
                    clang::SourceLocation startLoc = sizeExpr->getBeginLoc();
                    clang::SourceLocation endLoc = clang::Lexer::getLocForEndOfToken(sizeExpr->getEndLoc(), 0, SM, Context->getLangOpts());
                    clang::CharSourceRange range = clang::CharSourceRange::getCharRange(startLoc, endLoc);
                    size = clang::Lexer::getSourceText(range, SM, Context->getLangOpts()).str();
                }
                oneDimArrays.push_back({cName, t2Name, size, arrayType->getElementType().getAsString()});
            }
        }

        else if (varType->isAnyPointerType()) {
            std::string cName = VD->getNameAsString();
            std::string t2Name = MapName("Array", cName, declLine);
            std::string size = cName + "Size";
            oneDimArrays.push_back({cName, t2Name, size, varType->getPointeeType().getAsString()});
        }

        else {
            std::string cName = VD->getNameAsString();
            std::string t2Name = MapName("Variable", cName, declLine);
            variables.push_back({cName, t2Name, VD->getType().getAsString()});
        }
    }

    return true;
}

std::string VariableVisitor::MapName(std::string category, std::string cName, unsigned declLine){
    std::string t2Name;
    std::string metadataId;

    std::regex metadataIdPattern("(![0-9]+).*\\[" + cName + "\\] \\[line " + std::to_string(declLine) + "\\]");
    std::smatch metadataIdMatches;
    std::string::const_iterator metadataIdSearchStart(llFile.cbegin());
    while (std::regex_search(metadataIdSearchStart, llFile.cend(), metadataIdMatches, metadataIdPattern)) {
        metadataId = metadataIdMatches[1].str();
        break;
    }

    if(category == "Variable") {
        std::regex metadataUsagePattern("call void @llvm\\.dbg\\.value\\(metadata (?:!\\{.* %([a-zA-Z]+)\\}|!\\{.* %\"([0-9]+)\"\\}), i64 [0-9]+, metadata " + metadataId + "\\), !dbg ![0-9]+");
        std::smatch metadataUsageMatches;
        std::string::const_iterator metadataUsageSearchStart(llFile.cbegin());
        while (std::regex_search(metadataUsageSearchStart, llFile.cend(), metadataUsageMatches, metadataUsagePattern)) {
            if (!metadataUsageMatches[1].str().empty()) {
                t2Name = metadataUsageMatches[1].str();
            }
            else if (!metadataUsageMatches[2].str().empty()) {
                t2Name = metadataUsageMatches[2].str();
            }
            break;
        }
    }

    else if(category == "Array") {
        std::regex metadataUsagePattern("call void @llvm\\.dbg\\.(?:value|declare)\\(metadata (?:!\\{.*\\* %([a-zA-Z0-9_]+)\\}, i64 [0-9]+|!\\{\\[.* x .*\\]\\* %([a-zA-Z0-9_]+)\\}|!\\{.*\\* %\"([a-zA-Z0-9_]+)\"\\}), metadata " + metadataId + "\\), !dbg ![0-9]+");
        std::smatch metadataUsageMatches;
        std::string::const_iterator metadataUsageSearchStart(llFile.cbegin());
        while (std::regex_search(metadataUsageSearchStart, llFile.cend(), metadataUsageMatches, metadataUsagePattern)) {
            if (!metadataUsageMatches[1].str().empty()) {
                t2Name = metadataUsageMatches[1].str();
            }
            else if (!metadataUsageMatches[2].str().empty()) {
                t2Name = metadataUsageMatches[2].str();
            }
            else if (!metadataUsageMatches[3].str().empty()) {
                t2Name = metadataUsageMatches[3].str();
            }
            break;
        }
    }

    return "v" + t2Name;
}