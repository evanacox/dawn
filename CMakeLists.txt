cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
        dawn
        VERSION 0.0.1
        DESCRIPTION "A scappy compiler middle and back-end"
        HOMEPAGE_URL "https://github.com/evanacox/dawn"
        LANGUAGES CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Declare library ----

find_package(absl 20220623 REQUIRED)

add_subdirectory(lib)

# ---- Install rules ----

if (NOT CMAKE_SKIP_INSTALL_RULES)
    include(cmake/install-rules.cmake)
endif ()

# ---- Examples ----

if (PROJECT_IS_TOP_LEVEL)
    option(BUILD_EXAMPLES "Build examples tree." "${dawn_DEVELOPER_MODE}")
    if (BUILD_EXAMPLES)
        add_subdirectory(example)
    endif ()
endif ()

# ---- Developer mode ----

if (NOT DAWN_DEVELOPER_MODE)
    return()
elseif (NOT PROJECT_IS_TOP_LEVEL)
    message(
            AUTHOR_WARNING
            "Developer mode is intended for developers of dawn"
    )
endif ()

include(cmake/dev-mode.cmake)