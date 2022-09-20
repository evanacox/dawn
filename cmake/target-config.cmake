function(configure_dawn_target)
    set(options)
    set(oneValueArgs LIB_ALIAS)
    set(multiValueArgs FILES DEPENDS INCLUDES)
    cmake_parse_arguments(CONFIGURE_DAWN_TARGET "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if (CONFIGURE_DAWN_TARGET_LIB_ALIAS)
        add_library(${CONFIGURE_DAWN_TARGET_LIB_ALIAS} ALIAS ${ARGV0})
    endif ()

    set(TARGET ${ARGV0})

    target_include_directories(
            ${TARGET} ${warning_guard}
            PUBLIC
            ${CONFIGURE_DAWN_TARGET_INCLUDES}
    )

    target_compile_features(${TARGET} PUBLIC cxx_std_20)
    target_link_libraries(${TARGET} PUBLIC ${CONFIGURE_DAWN_TARGET_DEPENDS})

    separate_arguments(flagsAsList UNIX_COMMAND "${DAWN_CXX_FLAGS} ${DAWN_EXTRA_CXX_FLAGS}")
    target_compile_options(${TARGET} PRIVATE ${flagsAsList})

    set_target_properties(${TARGET} PROPERTIES
            CXX_CLANG_TIDY "${DAWN_CXX_CLANG_TIDY}"
            SOURCES "${CONFIGURE_DAWN_TARGET_FILES}")
endfunction()

function(add_dawn_library)
    set(options)
    set(oneValueArgs ALIAS EXPORT_NAME)
    set(multiValueArgs FILES)
    cmake_parse_arguments(ADD_DAWN_LIBRARY "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    add_library(${ARGV0} ${ADD_DAWN_LIBRARY_FILES})

    set_target_properties(
            ${ARGV0} PROPERTIES
            CXX_VISIBILITY_PRESET hidden
            VISIBILITY_INLINES_HIDDEN YES
            VERSION "${PROJECT_VERSION}"
            SOVERSION "${PROJECT_VERSION_MAJOR}"
            EXPORT_NAME ${ADD_DAWN_LIBRARY_EXPORT_NAME}
            OUTPUT_NAME ${ADD_DAWN_LIBRARY_EXPORT_NAME}
    )

    set(NEW_ARGS ${ARGN})

    if (ADD_DAWN_LIBRARY_ALIAS)
        list(APPEND NEW_ARGS LIB_ALIAS ${ADD_DAWN_LIBRARY_ALIAS})
    endif ()

    configure_dawn_target(${NEW_ARGS})
endfunction()

function(add_dawn_executable)
    add_executable(${ARGV0})

    configure_dawn_target(${ARGN})
endfunction()

