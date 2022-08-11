cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
        dawn
        VERSION 0.1.0
        DESCRIPTION "A scrappy compiler middle and back-end library"
        HOMEPAGE_URL "https://github.com/evanacox/dawn"
        LANGUAGES CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)
include(cmake/target-config.cmake)

# ---- Declare library ----

set(ABSL_PROPAGATE_CXX_STD ON)
add_subdirectory(vendor/abseil-cpp)

add_dawn_library(dawn_dawn
        ALIAS dawn::dawn
        EXPORT_NAME dawn
        DEPENDS absl::hash absl::strings absl::str_format absl::flat_hash_map
        INCLUDES
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/>"
        "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/export>"
        FILES
        ./dawn/ir/internal/type_manager.cc
        ./dawn/ir/module.cc
        ./dawn/ir/types.cc
        ./dawn/utility/assertions.cc
        ./dawn/utility/apint.cc
        ./dawn/ir/instructions.cc)

include(GenerateExportHeader)
generate_export_header(
        dawn_dawn
        BASE_NAME dawn
        EXPORT_MACRO_NAME DAWN_PUBLIC
        EXPORT_FILE_NAME export/dawn/shared_export.h
)

if (NOT BUILD_SHARED_LIBS)
    target_compile_definitions(dawn_dawn PUBLIC DAWN_STATIC_DEFINE)
endif ()

set_target_properties(
        dawn_dawn PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN YES
        VERSION "${PROJECT_VERSION}"
        SOVERSION "${PROJECT_VERSION_MAJOR}"
        EXPORT_NAME dawn
        OUTPUT_NAME dawn
)

add_subdirectory(tools)

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
