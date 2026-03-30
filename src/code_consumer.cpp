#include "../include/code_consumer.h"

void CodeConsumer::HandleTranslationUnit(clang::ASTContext &Context) {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
    std::error_code EC;
    llvm::raw_fd_ostream stream("./Artifacts/InstrumentedProgram.c", EC, llvm::sys::fs::OF_Text);
    TheRewriter.getEditBuffer(TheRewriter.getSourceMgr().getMainFileID()).write(stream);
    stream.close();

    std::ifstream inStream("./Artifacts/InstrumentedProgram.c");
    std::string fileContent((std::istreambuf_iterator<char>(inStream)), std::istreambuf_iterator<char>());
    inStream.close();
    std::stringstream newContent;
    newContent << "#include <stdio.h>\n"
                  "#include <signal.h>\n"
                  "#include <stdlib.h>\n"
                  "#include <string.h>\n"
                  "#include <unistd.h>\n"
                  "#include <stdbool.h>\n"
                  "\n";
    newContent << fileContent;
    std::ofstream outStream("./Artifacts/InstrumentedProgram.c");
    outStream << newContent.str();
    outStream.close();
}