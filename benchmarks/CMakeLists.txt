cmake_minimum_required(VERSION 3.14)

include(../cmake/project-is-top-level.cmake)

# ---- Dependencies ----

find_package(benchmark REQUIRED)
enable_testing()

# ---- Benches ----

add_executable(dawn_benches
        ./main.cc)
target_link_libraries(dawn_benches PRIVATE dawn::dawn benchmark::benchmark)
target_compile_features(dawn_benches PRIVATE cxx_std_20)
