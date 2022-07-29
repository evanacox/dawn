include(cmake/folders.cmake)

include(CTest)
if (DAWN_BUILD_TESTING)
    add_subdirectory(tests)
endif ()

option(DAWN_BUILD_MCSS_DOCS "Build documentation using Doxygen and m.css" OFF)
if (DAWN_BUILD_MCSS_DOCS)
    include(cmake/docs.cmake)
endif ()

option(DAWN_ENABLE_COVERAGE "Enable coverage support separate from CTest's" OFF)
if (DAWN_ENABLE_COVERAGE)
    include(cmake/coverage.cmake)
endif ()

set(
        FORMAT_PATTERNS
        lib/*.cc
        lib/*.h
        tools/*.cc
        tools/*.h
        tests/*.cc
        tests/*.h
        benchmarks/*.cc
        benchmarks/*.h
        include/*.h
        CACHE STRING
        "; separated patterns relative to the project source dir to format"
)

set(FORMAT_COMMAND clang-format CACHE STRING "Formatter to use")

add_custom_target(
        format-check
        COMMAND "${CMAKE_COMMAND}"
        -D "FORMAT_COMMAND=${FORMAT_COMMAND}"
        -D "PATTERNS=${FORMAT_PATTERNS}"
        -P "${PROJECT_SOURCE_DIR}/cmake/lint.cmake"
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        COMMENT "Linting the code"
        VERBATIM
)

add_custom_target(
        format-fix
        COMMAND "${CMAKE_COMMAND}"
        -D "FORMAT_COMMAND=${FORMAT_COMMAND}"
        -D "PATTERNS=${FORMAT_PATTERNS}"
        -D FIX=YES
        -P "${PROJECT_SOURCE_DIR}/cmake/lint.cmake"
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        COMMENT "Fixing the code"
        VERBATIM
)