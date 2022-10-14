include(cmake/folders.cmake)

option(DAWN_ENABLE_TESTS "Enable building tests" OFF)
include(CTest)
if (DAWN_ENABLE_TESTS)
    add_subdirectory(tests)
endif ()

option(DAWN_ENABLE_BENCHMARKS "Enable building benchmarks" OFF)
if (DAWN_ENABLE_BENCHMARKS)
    set(BENCHMARK_ENABLE_TESTING OFF)
    add_subdirectory(vendor/benchmark)
    add_subdirectory(benchmarks)
endif ()

option(DAWN_BUILD_MCSS_DOCS "Build documentation using Doxygen and m.css" OFF)
if (DAWN_BUILD_MCSS_DOCS)
    include(cmake/docs.cmake)
endif ()

option(DAWN_ENABLE_COVERAGE "Enable coverage support separate from CTest's" OFF)
if (DAWN_ENABLE_COVERAGE)
    include(cmake/coverage.cmake)
endif ()

include(cmake/format-targets.cmake)