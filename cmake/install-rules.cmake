if (PROJECT_IS_TOP_LEVEL)
    set(CMAKE_INSTALL_INCLUDEDIR include/dawn CACHE PATH "")
endif ()

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# find_package(<package>) call for consumers to find this project
set(package dawn)

install(
        DIRECTORY
        include/
        "${PROJECT_BINARY_DIR}/export/"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
        COMPONENT dawn_Development
)

install(
        TARGETS dawn_dawn
        EXPORT dawnTargets
        RUNTIME #
        COMPONENT dawn_Runtime
        LIBRARY #
        COMPONENT dawn_Runtime
        NAMELINK_COMPONENT dawn_Development
        ARCHIVE #
        COMPONENT dawn_Development
        INCLUDES #
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

write_basic_package_version_file(
        "${package}ConfigVersion.cmake"
        COMPATIBILITY SameMajorVersion
)

# Allow package maintainers to freely override the path for the configs
set(
        dawn_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/${package}"
        CACHE PATH "CMake package config location relative to the install prefix"
)
mark_as_advanced(dawn_INSTALL_CMAKEDIR)

install(
        FILES cmake/install-config.cmake
        DESTINATION "${dawn_INSTALL_CMAKEDIR}"
        RENAME "${package}Config.cmake"
        COMPONENT dawn_Development
)

install(
        FILES "${PROJECT_BINARY_DIR}/${package}ConfigVersion.cmake"
        DESTINATION "${dawn_INSTALL_CMAKEDIR}"
        COMPONENT dawn_Development
)

install(
        EXPORT dawnTargets
        NAMESPACE dawn::
        DESTINATION "${dawn_INSTALL_CMAKEDIR}"
        COMPONENT dawn_Development
)

if (PROJECT_IS_TOP_LEVEL)
    include(CPack)
endif ()
