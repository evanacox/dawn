cmake_minimum_required(VERSION 3.14)

project(dawnTests LANGUAGES CXX)

include(../cmake/project-is-top-level.cmake)
include(../cmake/folders.cmake)
include(../cmake/windows-set-path.cmake)

# ---- Dependencies ----

if (PROJECT_IS_TOP_LEVEL)
    find_package(dawn REQUIRED)
    enable_testing()
endif ()

# ---- Tests ----

add_executable(dawn_tests src/sanity.cc)
target_link_libraries(dawn_tests PRIVATE dawn::dawn gtest_main)
target_compile_features(dawn_tests PRIVATE cxx_std_20)

add_test(NAME dawn_tests COMMAND dawn_tests)
windows_set_path(dawn_tests dawn)

# ---- End-of-file commands ----

add_folders(Test)
