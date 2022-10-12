cmake_minimum_required(VERSION 3.14)

include(../cmake/project-is-top-level.cmake)

# ---- Dependencies ----

find_package(benchmark REQUIRED)
enable_testing()

# ---- Benches ----

add_dawn_executable(dawn_benches
        DEPENDS dawn::dawn benchmark::benchmark
        FILES
        ./main.cc)

add_dawn_executable(dawn_bench_playground
        DEPENDS dawn::dawn benchmark::benchmark
        FILES
        ./cfg_algorithms.cc)

# benchmarks need to not exit irregularly too
add_test(dawn_benches_test dawn_benches)