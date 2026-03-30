#include <fstream>
#include <sstream>
#include <iostream>

#include "clang/Tooling/Tooling.h"

#include "../include/loop_action.h"
#include "../include/variable_action.h"
#include "../include/code_action.h"

void FilterT2File(const std::string& t2FilePath) {
    bool flag = true;
    std::vector<std::string> requiredLines;
    std::string line;
    std::ifstream t2File(t2FilePath);
    while (getline(t2File, line)) {
        if (flag) {
            if (line.find(" := nondet();") != std::string::npos) {
                continue;
            }
            if (line.find("TO: ") != std::string::npos) {
                flag = false;
            }
            requiredLines.push_back(line);
        }
        else {
            requiredLines.push_back(line);
        }
    }
    t2File.close();
    std::ofstream outputFile(t2FilePath);
    for (const auto& requiredLine : requiredLines) {
        outputFile << requiredLine << std::endl;
    }
    outputFile.close();
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: ./Atropos <path/to/SourceCode> <path/to/TestCases>\n";
        return 1;
    }

    const std::string sourceCodePath = argv[1];
    std::ifstream stream(sourceCodePath);
    if (!stream) {
        std::cerr << strerror(errno) << ": " << sourceCodePath << std::endl;
        return 0;
    }
    std::stringstream buffer;
    buffer << stream.rdbuf();
    std::string sourceCode = buffer.str();

    std::filesystem::path testCasesPath = argv[2];

    std::filesystem::path directory = "./Artifacts";
    if (std::filesystem::exists(directory) && std::filesystem::is_directory(directory)) {
        for (const auto& file : std::filesystem::directory_iterator(directory)) {
            if (std::filesystem::is_regular_file(file)) {
                std::filesystem::remove(file);
            }
        }
    }

    // Ptr2Arr
    // ?

    // llvm2kittel
    clang::tooling::runToolOnCode(std::make_unique<LoopAction>(), sourceCode);

    std::vector<std::string> headerFiles;
    std::vector<std::string> macroDefinitions;
    std::istringstream srcStream(sourceCode);
    std::string line;
    while (std::getline(srcStream, line)) {
        if (line.find("#include") == 0) {
            headerFiles.push_back(line);
        }
        else if (line.find("#define") == 0) {
            macroDefinitions.push_back(line);
        }
    }

    for (const auto& file : std::filesystem::directory_iterator("./Artifacts")) {
        if (file.path().extension() == ".c") {
            std::ifstream inStream(file.path());
            std::string cFileContent((std::istreambuf_iterator<char>(inStream)), std::istreambuf_iterator<char>());
            inStream.close();
            std::stringstream newContent;
            for (const auto& item : headerFiles) {
                newContent << item << "\n";
            }
            for (const auto& item : macroDefinitions) {
                newContent << item << "\n";
            }
            newContent << cFileContent;
            std::ofstream outStream(file.path());
            outStream << newContent.str();
            outStream.close();

            std::string cFileName = file.path().stem().filename().string();
            std::string command = "docker run -it --platform linux/amd64 -v $(pwd):/Docker llvm2kittel /bin/bash -c '";
            command += "cd /Docker && ";
            command += "clang -Wall -Wextra -g -c -emit-llvm -O0 Artifacts/" + cFileName + ".c -o Artifacts/" + cFileName + ".bc && ";
            command += "llvm2kittel/build/llvm2kittel --dump-ll --no-slicing --eager-inline --t2 Artifacts/" + cFileName + ".bc > Artifacts/" + cFileName + ".t2'";
            int result = system(command.c_str());
            if (result != 0) {
                std::cerr << "Failed to generate .t2 file for " << file.path();
            }
            else {
                FilterT2File("./Artifacts/" + cFileName + ".t2");
            }
        }
    }

    // Mapping variable names
    for (const auto& file : std::filesystem::directory_iterator("./Artifacts")) {
        if (file.path().extension() == ".c") {
            std::string cFileName = file.path().stem().filename().string();

            size_t firstUnderscore = cFileName.find('_');
            size_t secondUnderscore = cFileName.find('_', firstUnderscore + 1);
            std::string functionName = cFileName.substr(firstUnderscore + 1, secondUnderscore - firstUnderscore - 1);
            std::string lineNumber = cFileName.substr(secondUnderscore + 1);

            std::ifstream llStream("./Artifacts/" + cFileName + ".ll");
            std::stringstream llBuffer;
            llBuffer << llStream.rdbuf();
            std::string llFile = llBuffer.str();

            std::ifstream t2Stream("./Artifacts/" + cFileName + ".t2");
            std::stringstream t2Buffer;
            t2Buffer << t2Stream.rdbuf();
            std::string t2File = t2Buffer.str();

            std::ifstream cStream(file.path());
            std::stringstream cBuffer;
            cBuffer << cStream.rdbuf();
            std::string cFile = cBuffer.str();

            clang::tooling::runToolOnCode(std::make_unique<VariableAction>(functionName, lineNumber, llFile, t2File), cFile);
        }
    }

    // Instrumenting code
    int loopCount = 0;
    for (const auto& file : std::filesystem::directory_iterator("./Artifacts")) {
        if (file.path().extension() == ".c") {
            loopCount = loopCount + 1;
        }
    }
    clang::tooling::runToolOnCode(std::make_unique<CodeAction>(loopCount), sourceCode);

    // Compiling instrumented program
    const char* command1 = "clang -o ./Artifacts/InstrumentedProgram ./Artifacts/InstrumentedProgram.c";
    int result1 = system(command1);
    if (result1 != 0) {
        std::cerr << "Failed to compile the instrumented program." << std::endl;
    }
    else {
        // Executing instrumented program on test data
        if (std::filesystem::exists(testCasesPath) && std::filesystem::is_directory(testCasesPath)) {
            for (const auto& file : std::filesystem::directory_iterator(testCasesPath)) {
                if (file.path().extension() == ".txt") {
                    std::cout << "\n" << file.path().stem().filename().string() << ":\n";
                    std::string command2 = "./Artifacts/InstrumentedProgram < \"" + file.path().string() + "\"";
                    system(command2.c_str());
                }
            }
        }
    }

    return 0;
}