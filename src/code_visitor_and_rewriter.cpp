#include "../include/code_visitor_and_rewriter.h"

bool CodeVisitorAndRewriter::VisitFunctionDecl(clang::FunctionDecl *FD) {
    clang::SourceManager &SM = TheRewriter.getSourceMgr();
    if (SM.isInMainFile(FD->getBeginLoc())) {
        if (FD->hasBody()) {
            if (isFirstFunction) {
                std::string textToInsert = "#define LOOP_COUNT " + std::to_string(loopCount) + "\n"
                                           "\n"
                                           "struct Loop {\n"
                                           "    bool is_initial_command;\n"
                                           "    char *t2_file_path;\n"
                                           "    char *txt_file_path;\n"
                                           "    char *program_state;\n"
                                           "    char *term_precondition;\n"
                                           "    char *nonterm_precondition;\n"
                                           "    char *unknown_precondition;\n"
                                           "};\n"
                                           "\n"
                                           "struct Loop loop_list[LOOP_COUNT];\n"
                                           "\n"
                                           "void initialize_loop(int id, char *t2_file_path, char *txt_file_path){\n"
                                           "    loop_list[id].is_initial_command = true;\n"
                                           "    loop_list[id].t2_file_path = t2_file_path;\n"
                                           "    loop_list[id].txt_file_path = txt_file_path;\n"
                                           "    loop_list[id].term_precondition = \"\";\n"
                                           "    loop_list[id].nonterm_precondition = \"\";\n"
                                           "    loop_list[id].unknown_precondition = \"\";\n"
                                           "}\n"
                                           "\n"
                                           "struct Pipe {\n"
                                           "    int inpipefd[2];\n"
                                           "    int outpipefd[2];\n"
                                           "    pid_t pid;\n"
                                           "};\n"
                                           "\n"
                                           "struct Pipe pipe_list[LOOP_COUNT];\n"
                                           "\n"
                                           "void initialize_pipe(struct Pipe *p) {\n"
                                           "    if (pipe(p->inpipefd) != 0 || pipe(p->outpipefd) != 0) {\n"
                                           "        perror(\"Failed to create pipe.\");\n"
                                           "        exit(EXIT_FAILURE);\n"
                                           "    }\n"
                                           "}\n"
                                           "\n"
                                           "void start(struct Pipe *p, const char *command) {\n"
                                           "    p->pid = fork();\n"
                                           "    if (p->pid == -1) {\n"
                                           "        perror(\"Failed to create new process.\");\n"
                                           "        exit(EXIT_FAILURE);\n"
                                           "    }\n"
                                           "    if (p->pid == 0) {\n"
                                           "        close(p->inpipefd[1]);\n"
                                           "        close(p->outpipefd[0]);\n"
                                           "        dup2(p->inpipefd[0], STDIN_FILENO);\n"
                                           "        dup2(p->outpipefd[1], STDOUT_FILENO);\n"
                                           "        close(p->inpipefd[0]);\n"
                                           "        close(p->outpipefd[1]);\n"
                                           "        execl(\"/bin/sh\", \"sh\", \"-c\", command, NULL);\n"
                                           "        exit(EXIT_FAILURE);\n"
                                           "    }\n"
                                           "    else {\n"
                                           "        close(p->inpipefd[0]);\n"
                                           "        close(p->outpipefd[1]);\n"
                                           "    }\n"
                                           "}\n"
                                           "\n"
                                           "void send(struct Pipe *p, const char *input) {\n"
                                           "    ssize_t result = write(p->inpipefd[1], input, strlen(input));\n"
                                           "    if (result == -1) {\n"
                                           "        perror(\"Failed to write to pipe.\");\n"
                                           "        exit(EXIT_FAILURE);\n"
                                           "    }\n"
                                           "    else if (result != strlen(input)) {\n"
                                           "        fprintf(stderr, \"Failed to completely write to pipe.\");\n"
                                           "    }\n"
                                           "}\n"
                                           "\n"
                                           "char *receive(struct Pipe *p) {\n"
                                           "    fd_set set;\n"
                                           "    FD_ZERO(&set);\n"
                                           "    FD_SET(p->outpipefd[0], &set);\n"
                                           "    struct timeval timeout;\n"
                                           "    timeout.tv_sec = 120;\n"
                                           "    timeout.tv_usec = 0;\n"
                                           "    int rv = select(p->outpipefd[0] + 1, &set, NULL, NULL, &timeout);\n"
                                           "    if (rv == -1) {\n"
                                           "        perror(\"Failed to select.\");\n"
                                           "        return NULL;\n"
                                           "    }\n"
                                           "    else if (rv == 0) {\n"
                                           "        return \"maybe\";\n"
                                           "    }\n"
                                           "    else {\n"
                                           "        static char output[10000];\n"
                                           "        memset(output, 0, sizeof(output));\n"
                                           "        ssize_t result = read(p->outpipefd[0], output, sizeof(output) - 1);\n"
                                           "        if (result == -1) {\n"
                                           "            perror(\"Failed to read from pipe.\");\n"
                                           "            return NULL;\n"
                                           "        }\n"
                                           "        output[result] = '\\0';\n"
                                           "        return output;\n"
                                           "    }\n"
                                           "}\n"
                                           "\n"
                                           "void terminate_pipe(struct Pipe *p) {\n"
                                           "    if (kill(p->pid, SIGTERM) == -1) {\n"
                                           "        perror(\"Failed to send SIGTERM\");\n"
                                           "    }\n"
                                           "}\n"
                                           "\n"
                                           "bool solve(char *precondition, char *program_state){\n"
                                           "    char *command;\n"
                                           "    asprintf(&command, \"chmod +x ./Solver.py && python3 ./Solver.py '%s' '%s'\", precondition, program_state);\n"
                                           "    int result = system(command);\n"
                                           "    if (result == 0) {\n"
                                           "        return true;\n"
                                           "    }\n"
                                           "    return false;\n"
                                           "}\n"
                                           "\n"
                                           "bool is_terminating(int id) {\n"
                                           "    if (solve(loop_list[id].term_precondition, loop_list[id].program_state)) {\n"
                                           "        return true;\n"
                                           "    }\n"
                                           "    return false;\n"
                                           "}\n"
                                           "\n"
                                           "bool is_nonterminating(int id) {\n"
                                           "    if (solve(loop_list[id].nonterm_precondition, loop_list[id].program_state)) {\n"
                                           "        return true;\n"
                                           "    }\n"
                                           "    return false;\n"
                                           "}\n"
                                           "\n"
                                           "void report(int id, char *situation){\n"
                                           "    printf(\"A/An %s situation is found for loop number %d:\\n\"\n"
                                           "           \"Program State: %s\\n\"\n"
                                           "           \"Terminating Precondition: %s\\n\"\n"
                                           "           \"Non-Terminating Precondition: %s\\n\"\n"
                                           "           \"Unknown Precondition: %s\\n\\n\",\n"
                                           "           situation,\n"
                                           "           id,\n"
                                           "           loop_list[id].program_state,\n"
                                           "           loop_list[id].term_precondition,\n"
                                           "           loop_list[id].nonterm_precondition,\n"
                                           "           loop_list[id].unknown_precondition);\n"
                                           "}\n"
                                           "\n"
                                           "void evaluate_program_state(int id) {\n"
                                           "    char *it;\n"
                                           "    int length;\n"
                                           "    const char *message1 = \"action (primal/dual/unknown/pos/neg/end): \";\n"
                                           "    const char *message2 = \"the specified constraints for positive and negative examples are incorrect\";\n"
                                           "\n"
                                           "    if(loop_list[id].is_initial_command){\n"
                                           "        initialize_pipe(&pipe_list[id]);\n"
                                           "        char *initial_command;\n"
                                           "        asprintf(&initial_command, \"cd ./muval && chmod +x dune && dune exec main -- -c ./config/solver/muval_prove_nonopt_tb_ar.json -p ltsterminter ../%s\", loop_list[id].t2_file_path);\n"
                                           "        start(&pipe_list[id], initial_command);\n"
                                           "        strdup(receive(&pipe_list[id]));\n"
                                           "        send(&pipe_list[id], \"\\n\");\n"
                                           "        strdup(receive(&pipe_list[id]));\n"
                                           "        loop_list[id].is_initial_command = false;\n"
                                           "    }\n"
                                           "\n"
                                           "    send(&pipe_list[id], \"pos\\n\");\n"
                                           "    strdup(receive(&pipe_list[id]));\n"
                                           "\n"
                                           "    char *update_command;\n"
                                           "    asprintf(&update_command, \"%s\\n\", loop_list[id].program_state);\n"
                                           "    send(&pipe_list[id], update_command);\n"
                                           "    strdup(receive(&pipe_list[id]));\n"
                                           "\n"
                                           "    send(&pipe_list[id], \"primal\\n\");\n"
                                           "    loop_list[id].term_precondition = strdup(receive(&pipe_list[id]));\n"
                                           "    if (strcmp(loop_list[id].term_precondition, \"maybe\") == 0) {\n"
                                           "        terminate_pipe(&pipe_list[id]);\n"
                                           "        initialize_pipe(&pipe_list[id]);\n"
                                           "        char *initial_command;\n"
                                           "        asprintf(&initial_command, \"cd ./muval && chmod +x dune && dune exec main -- -c ./config/solver/muval_prove_tb_ar.json -p ltsterminter ../%s\", loop_list[id].t2_file_path);\n"
                                           "        start(&pipe_list[id], initial_command);\n"
                                           "        strdup(receive(&pipe_list[id]));\n"
                                           "        send(&pipe_list[id], \"\\n\");\n"
                                           "        strdup(receive(&pipe_list[id]));\n"
                                           "    }\n"
                                           "    else {\n"
                                           "        if ((it = strstr(loop_list[id].term_precondition, message1)) == NULL) {\n"
                                           "            strdup(receive(&pipe_list[id]));\n"
                                           "        }\n"
                                           "        else {\n"
                                           "            ssize_t len = strlen(message1);\n"
                                           "            memmove(it, it + len, strlen(it + len) + 1);\n"
                                           "        }\n"
                                           "        if (strstr(loop_list[id].term_precondition, message2)) {\n"
                                           "            strcpy(loop_list[id].term_precondition, \"false\");\n"
                                           "        }\n"
                                           "        length = strlen(loop_list[id].term_precondition);\n"
                                           "        if (loop_list[id].term_precondition[length - 1] == '\\n') {\n"
                                           "            loop_list[id].term_precondition[length - 1] = '\\0';\n"
                                           "        }\n"
                                           "    }\n"
                                           "\n"
                                           "    send(&pipe_list[id], \"dual\\n\");\n"
                                           "    loop_list[id].nonterm_precondition = strdup(receive(&pipe_list[id]));\n"
                                           "    if (strcmp(loop_list[id].nonterm_precondition, \"maybe\") == 0) {\n"
                                           "        terminate_pipe(&pipe_list[id]);\n"
                                           "        initialize_pipe(&pipe_list[id]);\n"
                                           "        char *initial_command;\n"
                                           "        asprintf(&initial_command, \"cd ./muval && chmod +x dune && dune exec main -- -c ./config/solver/muval_prove_tb_ar.json -p ltsterminter ../%s\", loop_list[id].t2_file_path);\n"
                                           "        start(&pipe_list[id], initial_command);\n"
                                           "        strdup(receive(&pipe_list[id]));\n"
                                           "        send(&pipe_list[id], \"\\n\");\n"
                                           "        strdup(receive(&pipe_list[id]));\n"
                                           "    }\n"
                                           "    else {\n"
                                           "        if ((it = strstr(loop_list[id].nonterm_precondition, message1)) == NULL) {\n"
                                           "            strdup(receive(&pipe_list[id]));\n"
                                           "        }\n"
                                           "        else {\n"
                                           "            ssize_t len = strlen(message1);\n"
                                           "            memmove(it, it + len, strlen(it + len) + 1);\n"
                                           "        }\n"
                                           "        if (strstr(loop_list[id].nonterm_precondition, message2)) {\n"
                                           "            strcpy(loop_list[id].nonterm_precondition, \"false\");\n"
                                           "        }\n"
                                           "        length = strlen(loop_list[id].nonterm_precondition);\n"
                                           "        if (loop_list[id].nonterm_precondition[length - 1] == '\\n') {\n"
                                           "            loop_list[id].nonterm_precondition[length - 1] = '\\0';\n"
                                           "        }\n"
                                           "    }\n"
                                           "\n"
                                           "    send(&pipe_list[id], \"unknown\\n\");\n"
                                           "    loop_list[id].unknown_precondition = strdup(receive(&pipe_list[id]));\n"
                                           "    if (strcmp(loop_list[id].unknown_precondition, \"maybe\") == 0) {\n"
                                           "        terminate_pipe(&pipe_list[id]);\n"
                                           "        initialize_pipe(&pipe_list[id]);\n"
                                           "        char *initial_command;\n"
                                           "        asprintf(&initial_command, \"cd ./muval && chmod +x dune && dune exec main -- -c ./config/solver/muval_prove_tb_ar.json -p ltsterminter ../%s\", loop_list[id].t2_file_path);\n"
                                           "        start(&pipe_list[id], initial_command);\n"
                                           "        strdup(receive(&pipe_list[id]));\n"
                                           "        send(&pipe_list[id], \"\\n\");\n"
                                           "        strdup(receive(&pipe_list[id]));\n"
                                           "    }\n"
                                           "    else {\n"
                                           "        if ((it = strstr(loop_list[id].unknown_precondition, message1)) == NULL) {\n"
                                           "            strdup(receive(&pipe_list[id]));\n"
                                           "        }\n"
                                           "        else {\n"
                                           "            ssize_t len = strlen(message1);\n"
                                           "            memmove(it, it + len, strlen(it + len) + 1);\n"
                                           "        }\n"
                                           "        if (strstr(loop_list[id].unknown_precondition, message2)) {\n"
                                           "            strcpy(loop_list[id].unknown_precondition, \"false\");\n"
                                           "        }\n"
                                           "        length = strlen(loop_list[id].unknown_precondition);\n"
                                           "        if (loop_list[id].unknown_precondition[length - 1] == '\\n') {\n"
                                           "            loop_list[id].unknown_precondition[length - 1] = '\\0';\n"
                                           "        }\n"
                                           "    }\n"
                                           "}\n"
                                           "\n";
                TheRewriter.InsertTextBefore(FD->getBeginLoc(), textToInsert);
                isFirstFunction = false;
            }

            currentFunctionName = FD->getNameAsString();

            if (currentFunctionName == "read") {
                TheRewriter.ReplaceText(FD->getNameInfo().getSourceRange(), "readread");
            }
            if (currentFunctionName == "strcspn") {
                TheRewriter.ReplaceText(FD->getNameInfo().getSourceRange(), "strcspnstrcspn");
            }
            if (currentFunctionName == "strchr") {
                TheRewriter.ReplaceText(FD->getNameInfo().getSourceRange(), "strchrstrchr");
            }
            if (currentFunctionName == "waitpid") {
                TheRewriter.ReplaceText(FD->getNameInfo().getSourceRange(), "waitpidwaitpid");
            }
            if (currentFunctionName == "fgetc") {
                TheRewriter.ReplaceText(FD->getNameInfo().getSourceRange(), "fgetcfgetc");
            }
            if (currentFunctionName == "ffs") {
                TheRewriter.ReplaceText(FD->getNameInfo().getSourceRange(), "ffsffs");
            }

            if (currentFunctionName == "main") {
                mainFunctionStartLoc = FD->getBody()->getBeginLoc();
                mainFunctionStartLoc = mainFunctionStartLoc.getLocWithOffset(1);
            }
        }
        else {
            clang::SourceRange fullRange = FD->getSourceRange();
            clang::SourceLocation semicolonLoc = clang::Lexer::findLocationAfterToken(fullRange.getEnd(), clang::tok::semi, TheRewriter.getSourceMgr(), TheRewriter.getLangOpts(), false);
            fullRange.setEnd(semicolonLoc);
            TheRewriter.RemoveText(fullRange);
        }
    }
    return true;
}

bool CodeVisitorAndRewriter::VisitForStmt(clang::ForStmt *FS) {
    clang::SourceManager &SM = TheRewriter.getSourceMgr();
    clang::SourceLocation loopStartLoc = FS->getForLoc();
    std::string t2FilePath = "Artifacts/Loop_" + currentFunctionName + "_" + std::to_string(SM.getPresumedLoc(loopStartLoc).getLine()) + ".t2";
    std::string txtFilePath = "Artifacts/LoopInfo_" + currentFunctionName + "_" + std::to_string(SM.getPresumedLoc(loopStartLoc).getLine()) + ".txt";
    textToInsert1 = textToInsert1 + "initialize_loop(" + std::to_string(counter) + ", \"" + t2FilePath + "\", \"" + txtFilePath + "\");\n";

    std::string programState = GenerateProgramState(txtFilePath, counter);
    std::string textToInsert2 = "loop_list[" + std::to_string(counter) + "].program_state = malloc(1);\n"
                                "loop_list[" + std::to_string(counter) + "].program_state[0] = '\\0';\n"
                                + programState +
                                "int length_" + std::to_string(counter) + " = strlen(loop_list[" + std::to_string(counter) + "].program_state);\n"
                                "loop_list[" + std::to_string(counter) + "].program_state[length_" + std::to_string(counter) + " - 4] = '\\0';\n"
                                "while (true) {\n"
                                "   evaluate_program_state(" + std::to_string(counter) + ");\n"
                                "   if (is_terminating(" + std::to_string(counter) + ")) {\n"
                                "      report(" + std::to_string(counter) + ", \"terminating\");\n"
                                "      break;\n"
                                "   }\n"
                                "   else if (is_nonterminating(" + std::to_string(counter) + ")) {\n"
                                "      report(" + std::to_string(counter) + ", \"non-terminating\");\n"
                                "      abort();\n"
                                "   }\n"
                                "   else if ((strcmp(loop_list[" + std::to_string(counter) + "].term_precondition, \"maybe\") == 0) || (strcmp(loop_list[" + std::to_string(counter) + "].nonterm_precondition, \"maybe\") == 0)) {\n"
                                "       report(" + std::to_string(counter) + ", \"unknown\");\n"
                                "       abort();\n"
                                "   }\n"
                                "}\n";
    TheRewriter.InsertTextBefore(FS->getBeginLoc(), textToInsert2);

    counter++;

    if(counter == loopCount){
        TheRewriter.InsertTextAfterToken(mainFunctionStartLoc, textToInsert1);
    }

    return true;
}

bool CodeVisitorAndRewriter::VisitWhileStmt(clang::WhileStmt *WS) {
    clang::SourceManager &SM = TheRewriter.getSourceMgr();
    clang::SourceLocation loopStartLoc = WS->getWhileLoc();
    std::string t2FilePath = "Artifacts/Loop_" + currentFunctionName + "_" + std::to_string(SM.getPresumedLoc(loopStartLoc).getLine()) + ".t2";
    std::string txtFilePath = "Artifacts/LoopInfo_" + currentFunctionName + "_" + std::to_string(SM.getPresumedLoc(loopStartLoc).getLine()) + ".txt";
    textToInsert1 = textToInsert1 + "initialize_loop(" + std::to_string(counter) + ", \"" + t2FilePath + "\", \"" + txtFilePath + "\");\n";

    std::string programState = GenerateProgramState(txtFilePath, counter);
    std::string textToInsert2 = "loop_list[" + std::to_string(counter) + "].program_state = malloc(1);\n"
                                "loop_list[" + std::to_string(counter) + "].program_state[0] = '\\0';\n"
                                + programState +
                                "int length_" + std::to_string(counter) + " = strlen(loop_list[" + std::to_string(counter) + "].program_state);\n"
                                "loop_list[" + std::to_string(counter) + "].program_state[length_" + std::to_string(counter) + " - 4] = '\\0';\n"
                                "while (true) {\n"
                                "   evaluate_program_state(" + std::to_string(counter) + ");\n"
                                "   if (is_terminating(" + std::to_string(counter) + ")) {\n"
                                "      report(" + std::to_string(counter) + ", \"terminating\");\n"
                                "      break;\n"
                                "   }\n"
                                "   else if (is_nonterminating(" + std::to_string(counter) + ")) {\n"
                                "      report(" + std::to_string(counter) + ", \"non-terminating\");\n"
                                "      abort();\n"
                                "   }\n"
                                "   else if ((strcmp(loop_list[" + std::to_string(counter) + "].term_precondition, \"maybe\") == 0) || (strcmp(loop_list[" + std::to_string(counter) + "].nonterm_precondition, \"maybe\") == 0)) {\n"
                                "       report(" + std::to_string(counter) + ", \"unknown\");\n"
                                "       abort();\n"
                                "   }\n"
                                "}\n";
    TheRewriter.InsertTextBefore(WS->getBeginLoc(), textToInsert2);

    counter++;

    if(counter == loopCount){
        TheRewriter.InsertTextAfter(mainFunctionStartLoc, textToInsert1);
    }

    return true;
}

bool CodeVisitorAndRewriter::VisitDoStmt(clang::DoStmt *DS) {
    clang::SourceManager &SM = TheRewriter.getSourceMgr();
    clang::SourceLocation loopStartLoc = DS->getDoLoc();
    std::string t2FilePath = "Artifacts/Loop_" + currentFunctionName + "_" + std::to_string(SM.getPresumedLoc(loopStartLoc).getLine()) + ".t2";
    std::string txtFilePath = "Artifacts/LoopInfo_" + currentFunctionName + "_" + std::to_string(SM.getPresumedLoc(loopStartLoc).getLine()) + ".txt";
    textToInsert1 = textToInsert1 + "initialize_loop(" + std::to_string(counter) + ", \"" + t2FilePath + "\", \"" + txtFilePath + "\");\n";

    std::string programState = GenerateProgramState(txtFilePath, counter);
    std::string textToInsert2 = "loop_list[" + std::to_string(counter) + "].program_state = malloc(1);\n"
                                "loop_list[" + std::to_string(counter) + "].program_state[0] = '\\0';\n"
                                + programState +
                                "int length = strlen(loop_list[" + std::to_string(counter) + "].program_state);\n"
                                "loop_list[" + std::to_string(counter) + "].program_state[length - 4] = '\\0';\n"
                                "while (true) {\n"
                                "   evaluate_program_state(" + std::to_string(counter) + ");\n"
                                "   if (is_terminating(" + std::to_string(counter) + ")) {\n"
                                "      report(" + std::to_string(counter) + ", \"terminating\");\n"
                                "      break;\n"
                                "   }\n"
                                "   else if (is_nonterminating(" + std::to_string(counter) + ")) {\n"
                                "      report(" + std::to_string(counter) + ", \"non-terminating\");\n"
                                "      abort();\n"
                                "   }\n"
                                "   else if ((strcmp(loop_list[" + std::to_string(counter) + "].term_precondition, \"maybe\") == 0) || (strcmp(loop_list[" + std::to_string(counter) + "].nonterm_precondition, \"maybe\") == 0)) {\n"
                                "       report(" + std::to_string(counter) + ", \"unknown\");\n"
                                "       abort();\n"
                                "   }\n"
                                "}\n";
    TheRewriter.InsertTextBefore(DS->getBeginLoc(), textToInsert2);

    counter++;

    if(counter == loopCount){
        TheRewriter.InsertTextAfterToken(mainFunctionStartLoc, textToInsert1);
    }

    return true;
}

bool CodeVisitorAndRewriter::VisitVarDecl(clang::VarDecl *VD) {
    if (VD->hasInit()) {
        clang::Expr *init = VD->getInit();
        if (clang::CallExpr *callExpr = dyn_cast<clang::CallExpr>(init)) {
            if (clang::FunctionDecl *functionDecl = callExpr->getDirectCallee()) {
                std::string functionName = functionDecl->getNameAsString();
                if (functionName.find("__VERIFIER_nondet_") != std::string::npos) {
                    std::string type;
                    std::string formatSpecifier;
                    std::string variableName = VD->getNameAsString();
                    if (functionName == "__VERIFIER_nondet_int") {
                        type = "int";
                        formatSpecifier = "%d";
                    }
                    if (functionName == "__VERIFIER_nondet_char") {
                        type = "char";
                        formatSpecifier = "%c";
                    }
                    if (functionName == "__VERIFIER_nondet_uint") {
                        type = "unsigned int";
                        formatSpecifier = "%u";
                    }
                    if (functionName == "__VERIFIER_nondet_ushort") {
                        type = "unsigned short";
                        formatSpecifier = "%hu";
                    }
                    if (functionName == "__VERIFIER_nondet_uchar") {
                        type = "unsigned char";
                        formatSpecifier = "%c";
                    }
                    std::stringstream stream;
                    stream << type << " " << variableName << ";\nscanf(\"" << formatSpecifier << "\", &" << variableName << ")";
                    TheRewriter.ReplaceText(VD->getSourceRange(), stream.str());
                }
            }
        }
    }
    return true;
}

bool CodeVisitorAndRewriter::VisitBinaryOperator(clang::BinaryOperator *BO) {
    if (BO->isAssignmentOp()) {
        clang::Expr *rightExpr = BO->getRHS();
        if (clang::CallExpr *callExpr = dyn_cast<clang::CallExpr>(rightExpr)) {
            if (clang::FunctionDecl *functionDecl = callExpr->getDirectCallee()) {
                std::string functionName = functionDecl->getNameAsString();
                if (functionName.find("__VERIFIER_nondet_") != std::string::npos) {
                    std::string formatSpecifier;
                    std::string variableName = dyn_cast<clang::DeclRefExpr>(BO->getLHS()->IgnoreImpCasts())->getNameInfo().getAsString();
                    if (functionName == "__VERIFIER_nondet_int") {
                        formatSpecifier = "%d";
                    }
                    if (functionName == "__VERIFIER_nondet_char") {
                        formatSpecifier = "%c";
                    }
                    if (functionName == "__VERIFIER_nondet_uint") {
                        formatSpecifier = "%u";
                    }
                    if (functionName == "__VERIFIER_nondet_ushort") {
                        formatSpecifier = "%hu";
                    }
                    if (functionName == "__VERIFIER_nondet_uchar") {
                        formatSpecifier = "%c";
                    }
                    std::stringstream stream;
                    stream << "scanf(\"" << formatSpecifier << "\", &" << variableName << ")";
                    TheRewriter.ReplaceText(rightExpr->getSourceRange(), stream.str());
                }
            }
        }
    }
    return true;
}

bool CodeVisitorAndRewriter::VisitIfStmt(clang::IfStmt *IS) {
    clang::Expr *condition = IS->getCond();
    if (condition) {
        isInsideIfCondition = true;
        hasNondetCall = false;
        TraverseStmt(condition);
        if (hasNondetCall) {
            TheRewriter.InsertTextBefore(IS->getIfLoc(), "int newVar;\nscanf(\"%d\", &newVar);\n");
        }
        isInsideIfCondition = false;
    }
    return true;
}

bool CodeVisitorAndRewriter::VisitCallExpr(clang::CallExpr *CE) {
    if (isInsideIfCondition) {
        clang::FunctionDecl *functionDecl = CE->getDirectCallee();
        if (functionDecl && functionDecl->getName() == "__VERIFIER_nondet_int") {
            hasNondetCall = true;
            TheRewriter.ReplaceText(CE->getSourceRange(), "newVar");
        }
    }
    return true;
}

bool CodeVisitorAndRewriter::VisitReturnStmt(clang::ReturnStmt *RS) {
    if (RS->getRetValue()) {
        clang::CallExpr* call = dyn_cast<clang::CallExpr>(RS->getRetValue()->IgnoreImpCasts());
        if (call && isa<clang::FunctionDecl>(call->getCalleeDecl())) {
            clang::FunctionDecl *functionDecl = cast<clang::FunctionDecl>(call->getCalleeDecl());
            if (functionDecl->getName() == "__VERIFIER_nondet_int") {
                TheRewriter.InsertTextBefore(RS->getBeginLoc(), "int newVar;\n");
                TheRewriter.ReplaceText(RS->getRetValue()->getSourceRange(), "scanf(\"%d\", &newVar)");
            }
        }
    }
    return true;
}

bool CodeVisitorAndRewriter::VisitDeclRefExpr(clang::DeclRefExpr *DRE){
    if (DRE->getNameInfo().getAsString() == "read") {
        TheRewriter.ReplaceText(DRE->getNameInfo().getSourceRange(), "readread");
    }
    if (DRE->getNameInfo().getAsString() == "strcspn") {
        TheRewriter.ReplaceText(DRE->getNameInfo().getSourceRange(), "strcspnstrcspn");
    }
    if (DRE->getNameInfo().getAsString() == "strchr") {
        TheRewriter.ReplaceText(DRE->getNameInfo().getSourceRange(), "strchrstrchr");
    }
    if (DRE->getNameInfo().getAsString() == "waitpid") {
        TheRewriter.ReplaceText(DRE->getNameInfo().getSourceRange(), "waitpidwaitpid");
    }
    if (DRE->getNameInfo().getAsString() == "fgetc") {
        TheRewriter.ReplaceText(DRE->getNameInfo().getSourceRange(), "fgetcfgetc");
    }
    if (DRE->getNameInfo().getAsString() == "ffs") {
        TheRewriter.ReplaceText(DRE->getNameInfo().getSourceRange(), "ffsffs");
    }
    return true;
}

std::string CodeVisitorAndRewriter::GenerateProgramState(std::string txtFilePath, int counter){
    std::string programState_variable;
    std::string programState_oneDimArray;
    std::string programState_twoDimArray;
    std::string programState_structure;

    std::string condition;
    std::string variables;

    std::string array_cName;
    std::string array_t2Name;
    std::string array_size;

    std::ifstream file(txtFilePath);
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream stream(line);
        std::string segment;
        std::vector<std::string> segments;
        while (std::getline(stream, segment, ',')) {
            segments.push_back(segment);
        }

        if (segments[0] == "Variable") {
            std::string formatSpecifier;
            if (segments[3] == "float") {
                formatSpecifier = "%g";
            }
            else if (segments[3] == "long") {
                formatSpecifier = "%ld";
            }
            else if (segments[3] == "int") {
                formatSpecifier = "%d";
            }
            else if (segments[3] == "char") {
                formatSpecifier = "%c";
            }
            else if (segments[3] == "unsigned long") {
                formatSpecifier = "%lu";
            }
            else if (segments[3] == "unsigned int") {
                formatSpecifier = "%u";
            }
            else if (segments[3] == "unsigned short") {
                formatSpecifier = "%hu";
            }
            else if (segments[3] == "unsigned char") {
                formatSpecifier = "%c";
            }
            condition += segments[2] + " = " + formatSpecifier + " /\\\\ ";
            variables += ", " + segments[1];
        }
        else if (segments[0] == "OneDimArray") {
            std::string formatSpecifier;
            if (segments[4] == "int") {
                formatSpecifier = "%d";
            }
            else if (segments[4] == "char") {
                formatSpecifier = "%c";
            }
            else{
                array_cName = segments[1];
                array_t2Name = segments[2];
                array_size = segments[3];
                continue;
            }
            programState_oneDimArray += "for (int i = 0; i < " + segments[3] + "; i++) {\n"
                                        "   char *temporary_variable;\n"
                                        "   asprintf(&temporary_variable, \"getelement(" + segments[2] + ", %d) = " + formatSpecifier + " /\\\\ \", i, " + segments[1] + "[i]);\n"
                                        "   char *new_program_state = realloc(loop_list[" + std::to_string(counter) + "].program_state, strlen(loop_list[" + std::to_string(counter) + "].program_state) + strlen(temporary_variable) + 1);\n"
                                        "   loop_list[" + std::to_string(counter) + "].program_state = new_program_state;\n"
                                        "   strcat(loop_list[" + std::to_string(counter) + "].program_state, temporary_variable);\n"
                                        "   free(temporary_variable);\n"
                                        "}\n";
        }
        else if (segments[0] == "twoDimArray") {
            std::string formatSpecifier;
            if (segments[5] == "int") {
                formatSpecifier = "%d";
            }
            programState_twoDimArray += "for (int i = 0; i < " + segments[3] + "; i++) {\n"
                                        "   for (int j = 0; j < " + segments[4] + "; j++) {\n"
                                        "       char *temporary_variable;\n"
                                        "       asprintf(&temporary_variable, \"getelement(getelement(" + segments[2] + ", %d), %d) = " + formatSpecifier + " /\\\\ \", i, j, " + segments[1] + "[i][j]);\n"
                                        "       char *new_program_state = realloc(loop_list[" + std::to_string(counter) + "].program_state, strlen(loop_list[" + std::to_string(counter) + "].program_state) + strlen(temporary_variable) + 1);\n"
                                        "       loop_list[" + std::to_string(counter) + "].program_state = new_program_state;\n"
                                        "       strcat(loop_list[" + std::to_string(counter) + "].program_state, temporary_variable);\n"
                                        "       free(temporary_variable);\n"
                                        "   }\n"
                                        "}\n";
        }
        else if (segments[0] == "Structure") {
            std::string formatSpecifier;
            if (segments[5] == "int") {
                formatSpecifier = "%d";
            }
            programState_structure += "for (int i = 0; i < " + array_size + "; i++) {\n"
                                      "   char *temporary_variable;\n"
                                      "   asprintf(&temporary_variable, \"getfield(" + segments[2] + ", getelement(" + array_t2Name + ", %d), " + segments[4] + ") = " + formatSpecifier + " /\\\\ \", i, " + array_cName + "[i]." + segments[3] + ");\n"
                                      "   char *new_program_state = realloc(loop_list[" + std::to_string(counter) + "].program_state, strlen(loop_list[" + std::to_string(counter) + "].program_state) + strlen(temporary_variable) + 1);\n"
                                      "   loop_list[" + std::to_string(counter) + "].program_state = new_program_state;\n"
                                      "   strcat(loop_list[" + std::to_string(counter) + "].program_state, temporary_variable);\n"
                                      "   free(temporary_variable);\n"
                                      "}\n";
        }
    }
    file.close();

    programState_variable = "char *temporary_variable_" + std::to_string(counter) + ";\n"
                            "asprintf(&temporary_variable_" + std::to_string(counter) + ", \"" + condition + "\"" + variables + ");\n"
                            "char *new_program_state_" + std::to_string(counter) + " = realloc(loop_list[" + std::to_string(counter) + "].program_state, strlen(loop_list[" + std::to_string(counter) + "].program_state) + strlen(temporary_variable_" + std::to_string(counter) + ") + 1);\n"
                            "loop_list[" + std::to_string(counter) + "].program_state = new_program_state_" + std::to_string(counter) + ";\n"
                            "strcat(loop_list[" + std::to_string(counter) + "].program_state, temporary_variable_" + std::to_string(counter) + ");\n"
                            "free(temporary_variable_" + std::to_string(counter) + ");\n";

    return programState_variable + programState_oneDimArray + programState_twoDimArray + programState_structure;
}