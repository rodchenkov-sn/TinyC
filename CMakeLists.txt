cmake_minimum_required(VERSION 3.22)

project(TinyC)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE tcc_sources RELATIVE ${PROJECT_SOURCE_DIR} "src/tcc/*.cpp")
file(GLOB_RECURSE grammar_generated_sources RELATIVE ${PROJECT_SOURCE_DIR} "generated/*.cpp")

include_directories("${PROJECT_SOURCE_DIR}/src/tcc")
include_directories("${PROJECT_SOURCE_DIR}/generated")

find_package(antlr4-runtime REQUIRED)
include_directories(${ANTLR4_INCLUDE_DIR})

find_package(LLVM REQUIRED CONFIG)
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(llvm_libs
        Analysis
        Core
        ExecutionEngine
        InstCombine
        Object
        RuntimeDyld
        ScalarOpts
        Support
        native
        )

add_executable(tcc ${tcc_sources} ${grammar_generated_sources})

target_link_libraries(tcc antlr4_static ${llvm_libs})
