cmake_minimum_required(VERSION 3.27)
project(Atropos)

# Set the C++ standard to C++20
set(CMAKE_CXX_STANDARD 23)

# Find LLVM package
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Include LLVM headers
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Find Clang package
find_package(Clang REQUIRED CONFIG)
message(STATUS "Using ClangConfig.cmake in: ${CLANG_DIR}")

# Include Clang headers
include_directories(${CLANG_INCLUDE_DIRS})

# Find Z3 package
find_package(Z3 REQUIRED CONFIG)
message(STATUS "Found Z3 ${Z3_VERSION_STRING}")
message(STATUS "Using Z3Config.cmake in: ${Z3_DIR}")

# Include Z3 headers
include_directories(${Z3_INCLUDE_DIRS})

add_executable(Atropos src/main.cpp
        include/loop_action.h
        src/loop_action.cpp
        include/loop_consumer.h
        src/loop_consumer.cpp
        include/loop_visitor.h
        src/loop_visitor.cpp
        include/variable_action.h
        src/variable_action.cpp
        include/variable_consumer.h
        src/variable_consumer.cpp
        include/variable_visitor.h
        src/variable_visitor.cpp
        include/code_action.h
        src/code_action.cpp
        include/code_consumer.h
        src/code_consumer.cpp
        include/code_visitor_and_rewriter.h
        src/code_visitor_and_rewriter.cpp)

# Link against specific LLVM and Clang libraries
target_link_libraries(Atropos
        LLVM
        clangAST
        clangFrontend
        clangTooling
        ${Z3_LIBRARIES})