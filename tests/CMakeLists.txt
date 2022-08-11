cmake_minimum_required(VERSION 3.14)

include(../cmake/project-is-top-level.cmake)

# ---- Dependencies ----

if (PROJECT_IS_TOP_LEVEL)
    enable_testing()
endif ()

# ---- Tests ----

include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_dawn_executable(dawn_unittests
        DEPENDS dawn::dawn GTest::gtest GTest::gtest_main
        FILES
        ./sanity.cc
        ../dawn/ir/internal/type_manager.test.cc
        ../dawn/ir/types.test.cc
        ../dawn/utility/assertions.test.cc
        ../dawn/ir/instruction.test.cc
        ../dawn/ir/instructions.test.cc
        ../dawn/utility/apint.test.cc)

include(GoogleTest)
gtest_discover_tests(dawn_unittests)
