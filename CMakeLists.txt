cmake_minimum_required(VERSION 3.22)

project(TinyC)

set(CMAKE_CXX_STANDARD 17)

find_package(antlr4-generator REQUIRED)

set(ANTLR4_JAR_LOCATION "${PROJECT_SOURCE_DIR}/vendor/antlr4/antlr-4.10.1-complete.jar")
antlr4_generate(
        antlr_generated_sources
        ${PROJECT_SOURCE_DIR}/TinyC.g4
        BOTH
        FALSE
        TRUE
)

include_directories(${ANTLR4_INCLUDE_DIR_antlr_generated_sources})

file(GLOB_RECURSE tcc_sources RELATIVE ${PROJECT_SOURCE_DIR} "src/tcc/*.cpp")
include_directories("${PROJECT_SOURCE_DIR}/src/tcc")

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

add_executable(tcc
        ${tcc_sources}
        ${grammar_generated_sources}
        ${ANTLR4_SRC_FILES_antlr_generated_sources}
        )

target_link_libraries(tcc antlr4_static ${llvm_libs})
