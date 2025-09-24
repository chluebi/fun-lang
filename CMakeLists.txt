cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(LLVMAliasAnalysis CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(LLVM 17 REQUIRED CONFIG
  COMPONENTS
    Analysis  # For AAManager, PassManagers
    Core      # For basic LLVM data structures
    IR        # For Module, Function, Instruction, etc.
    IRReader  # For parseIRFile
    Option    # For command-line parsing (cl::opt)
    Passes    # For PassBuilder
    Support   # For raw_ostream (outs(), errs()), SourceMgr, etc.
)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")



# interpreter tests
add_executable(interpreter_tests src/test_interpreter.cpp)
target_compile_options(interpreter_tests PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)
target_link_libraries(interpreter_tests PRIVATE LLVM)


# Main interpreter executable
add_executable(interpreter src/main.cpp src/parser.cpp src/lexer.cpp src/interpreter.cpp src/runner.cpp)
target_compile_options(interpreter PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)
target_link_libraries(interpreter PRIVATE LLVM)