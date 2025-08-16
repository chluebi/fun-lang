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


add_executable(interpreter src/interpreter.cpp)
target_link_libraries(interpreter PRIVATE LLVM)