if(WIN32)
        set(VCPKG_TARGET_TRIPLET "x64-windows-static")
endif()

cmake_minimum_required(VERSION 3.18)

project(TinyC)

set(CMAKE_CXX_STANDARD 17)


# ANTLR

find_package(antlr4-runtime REQUIRED)
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
include_directories(${ANTLR4_INCLUDE_DIR})
add_compile_definitions("ANTLR4CPP_STATIC")


# LLVM

find_package(LLVM REQUIRED)
include_directories(${LLVM_INCLUDE_DIRS})
llvm_map_components_to_libnames(llvm_libs Passes)
add_definitions(${LLVM_DEFINITIONS})


# other libs

find_package(spdlog REQUIRED)
find_package(argparse REQUIRED)


# sources

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


add_executable(
        tcc
        ${tcc_sources}
        ${ANTLR4_SRC_FILES_antlr_generated_sources}
)

target_link_libraries(
        tcc
        antlr4_static
        ${llvm_libs}
        spdlog::spdlog
        argparse::argparse
)


if(WIN32)
        add_compile_definitions(NOGDI)

        set_property(TARGET tcc PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
