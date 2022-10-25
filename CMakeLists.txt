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

set(ABSL_USE_SYSTEM_INCLUDES ON)
set(ABSL_PROPAGATE_CXX_STD ON)
set(ABSL_BUILD_TESTING OFF)
add_subdirectory(vendor/abseil-cpp)

# ---- Declare library ----

add_dawn_library(dawn_dawn
        ALIAS dawn::dawn
        EXPORT_NAME dawn
        DEPENDS absl::hash absl::strings absl::str_format absl::flat_hash_map
        INCLUDES "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/>" "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/export>"
        FILES
        ./dawn/ir/module.cc
        ./dawn/ir/types.cc
        ./dawn/ir/instructions.cc
        ./dawn/ir/instructions.cc
        ./dawn/ir/instruction.cc
        ./dawn/ir/basic_block.cc
        ./dawn/ir/ir_builder.cc
        ./dawn/ir/function.cc
        ./dawn/ir/constant.cc
        ./dawn/ir/ir_writer.cc
        ./dawn/ir/ir_reader.cc
        ./dawn/ir/inst_visitor.cc
        ./dawn/ir/internal/type_manager.cc
        ./dawn/ir/internal/function_manager.cc
        ./dawn/ir/internal/instruction_manager.cc
        ./dawn/ir/internal/string_pool.cc
        ./dawn/ir/internal/constant_pool.cc
        ./dawn/utility/file.cc
        ./dawn/utility/assertions.cc
        ./dawn/utility/apint.cc
        ./dawn/utility/version.cc
        ./dawn/analysis/cfg_analysis.cc
        ./dawn/analysis/analysis_passes.cc
        ./dawn/analysis/analysis_manager.cc
        ./dawn/passes/pass.cc
        ./dawn/vm/vm.cc
        ./dawn/vm/interpret.cc
        ./dawn/vm/stack.cc)

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
        VERSION "${PROJECT_VERSION}"
        SOVERSION "${PROJECT_VERSION_MAJOR}"
        EXPORT_NAME dawn
        OUTPUT_NAME dawn
)

set(DAWN_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(DAWN_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(DAWN_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
configure_file(./dawn/version_config.h.in "${PROJECT_BINARY_DIR}/export/dawn/version_config.h")

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
