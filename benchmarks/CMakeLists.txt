cmake_minimum_required(VERSION 3.14)

include(../cmake/project-is-top-level.cmake)

# ---- Dependencies ----

enable_testing()

# ---- Benches ----

# see https://github.com/google/benchmark/issues/202
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(DAWN_BENCHMARK_EXTRA_LIBS "Shlwapi.lib")
else ()
    set(DAWN_BENCHMARK_EXTRA_LIBS)
endif ()

add_dawn_executable(dawn_benches
        DEPENDS dawn::dawn benchmark::benchmark ${DAWN_BENCHMARK_EXTRA_LIBS}
        FILES
        ./main.cc)

add_dawn_executable(dawn_bench_playground
        DEPENDS dawn::dawn benchmark::benchmark
        FILES
        ./cfg_algorithms.cc)

# benchmarks need to not exit irregularly too
add_test(dawn_benches_test dawn_benches)