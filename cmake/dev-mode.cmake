include(cmake/folders.cmake)

include(CTest)
if (BUILD_TESTING)
    find_package(GTest 1.11.0 REQUIRED)
    add_subdirectory(tests)

    find_package(benchmark 1.6.0 REQUIRED)
    add_subdirectory(benchmarks)
endif ()

option(BUILD_MCSS_DOCS "Build documentation using Doxygen and m.css" OFF)
if (BUILD_MCSS_DOCS)
    include(cmake/docs.cmake)
endif ()

option(ENABLE_COVERAGE "Enable coverage support separate from CTest's" OFF)
if (ENABLE_COVERAGE)
    include(cmake/coverage.cmake)
endif ()

if (CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    include(cmake/open-cpp-coverage.cmake OPTIONAL)
endif ()

include(cmake/lint-targets.cmake)

add_folders(Project)
