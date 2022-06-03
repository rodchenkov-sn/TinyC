cmake_minimum_required(VERSION 3.22)

project(TinyC)

set(CMAKE_CXX_STANDARD 17)

find_package(antlr4-generator REQUIRED)

set(ANTLR4_JAR_LOCATION "${PROJECT_SOURCE_DIR}/vendor/antlr4/antlr-4.9.3-complete.jar")
antlr4_generate(
        antlr_generated_sources
        ${PROJECT_SOURCE_DIR}/grammar/TinyC.g4
        BOTH
        FALSE
        TRUE
)

source_group("generated" FILES "${ANTLR4_SRC_FILES_antlr_generated_sources}")

include_directories(${ANTLR4_INCLUDE_DIR_antlr_generated_sources})


# from: https://stackoverflow.com/a/33813154
set(tcc_src_root "${PROJECT_SOURCE_DIR}/src/tcc")

file(
        GLOB_RECURSE tcc_sources 
        LIST_DIRECTORIES false
        "${tcc_src_root}/*.cpp"
        "${tcc_src_root}/*.h"
)

foreach(_source IN ITEMS ${tcc_sources})
        get_filename_component(_source_path "${_source}" PATH)
        file(RELATIVE_PATH _source_path_rel "${tcc_src_root}" "${_source_path}")
        string(REPLACE "/" "\\" _group_path "${_source_path_rel}")
        source_group("${_group_path}" FILES "${_source}")
endforeach()


include_directories("${PROJECT_SOURCE_DIR}/src/tcc")

find_package(antlr4-runtime REQUIRED)
include_directories(${ANTLR4_INCLUDE_DIR})

find_package(LLVM REQUIRED CONFIG)
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(llvm_libs Passes)

include_directories("${PROJECT_SOURCE_DIR}/vendor/argparse/include")

find_package(spdlog REQUIRED)

add_executable(
        tcc
        ${tcc_sources}
        ${ANTLR4_SRC_FILES_antlr_generated_sources}
)

target_link_libraries(tcc antlr4_shared ${llvm_libs} spdlog::spdlog)

if(WIN32)
        add_compile_definitions(NOGDI)
endif()
