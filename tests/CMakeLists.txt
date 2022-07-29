cmake_minimum_required(VERSION 3.14)

include(../cmake/project-is-top-level.cmake)

# ---- Dependencies ----

if (PROJECT_IS_TOP_LEVEL)
    enable_testing()
endif ()

# ---- Tests ----

add_executable(dawn_test
        ../lib/ir/internal/type_manager.test.cc
        ../lib/ir/types.test.cc
        ../lib/utility/assertions.test.cc
        ./sanity.cc)
target_link_libraries(dawn_test PRIVATE dawn::dawn gtest gtest_main)
target_compile_features(dawn_test PRIVATE cxx_std_20)

include(GoogleTest)
gtest_discover_tests(dawn_test)
