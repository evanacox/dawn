
set(DAWN_FORMAT_PATTERNS
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

option(DAWN_FORMAT_COMMAND "Formatter to use" "clang-tidy")

add_custom_target(
        format-check
        COMMAND "${CMAKE_COMMAND}"
        -D "FORMAT_COMMAND=${DAWN_FORMAT_COMMAND}"
        -D "PATTERNS=${DAWN_FORMAT_PATTERNS}"
        -P "${PROJECT_SOURCE_DIR}/cmake/format.cmake"
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        COMMENT "Linting the code"
        VERBATIM
)

add_custom_target(
        format-fix
        COMMAND "${CMAKE_COMMAND}"
        -D "FORMAT_COMMAND=${DAWN_FORMAT_COMMAND}"
        -D "PATTERNS=${DAWN_FORMAT_PATTERNS}"
        -D FIX=YES
        -P "${PROJECT_SOURCE_DIR}/cmake/format.cmake"
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        COMMENT "Fixing the code"
        VERBATIM
)