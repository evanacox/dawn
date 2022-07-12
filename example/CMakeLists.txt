cmake_minimum_required(VERSION 3.14)

project(dawnExamples CXX)

include(../cmake/project-is-top-level.cmake)
include(../cmake/folders.cmake)

if (PROJECT_IS_TOP_LEVEL)
    find_package(dawn REQUIRED)
endif ()

add_custom_target(run-examples)

function(add_example NAME)
    add_executable("${NAME}-example" "${NAME}.cc")
    target_link_libraries("${NAME}-example" PRIVATE dawn::dawn)
    target_compile_features("${NAME}-example" PRIVATE cxx_std_20)
    add_custom_target("run-${NAME}-example" COMMAND "${NAME}-example" VERBATIM)
    add_dependencies("run-${NAME}-example" "${NAME}-example")
    add_dependencies(run-examples "run-${NAME}-example")
endfunction()

add_folders(Example)
