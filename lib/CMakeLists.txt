add_library(dawn_dawn
        config/version.cc)

if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_definitions(dawn_dawn PRIVATE DAWN_COMPILER_GNULIKE)
endif ()

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(dawn_dawn PRIVATE DAWN_OPTIMIZED_BUILD)
endif ()

target_compile_definitions(dawn_dawn PRIVATE "DAWN_VERSION_STRING='${PROJECT_VERSION}'")
target_link_libraries(dawn_dawn PRIVATE absl::strings)

include(GenerateExportHeader)
generate_export_header(
        dawn_dawn
        BASE_NAME dawn_dawn
        EXPORT_FILE_NAME export/dawn/dawn_export.h
        CUSTOM_CONTENT_FROM_VARIABLE pragma_suppress_c4251)

if (NOT BUILD_SHARED_LIBS)
    target_compile_definitions(dawn_dawn PUBLIC DAWN_STATIC_DEFINE)
endif ()

set_target_properties(dawn_dawn PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN YES
        VERSION "${PROJECT_VERSION}"
        SOVERSION "${PROJECT_VERSION_MAJOR}"
        EXPORT_NAME dawn_dawn
        OUTPUT_NAME dawn_dawn)

target_include_directories(
        dawn_dawn ${warning_guard}
        PUBLIC
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>")

target_include_directories(
        dawn_dawn
        PRIVATE
        "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>")

add_library(dawn::dawn ALIAS dawn_dawn)

target_include_directories(
        dawn_dawn SYSTEM
        PUBLIC
        "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/export>")

target_compile_features(dawn_dawn PUBLIC cxx_std_20)