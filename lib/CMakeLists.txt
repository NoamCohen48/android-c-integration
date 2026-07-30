cmake_minimum_required(VERSION 3.16)

project(mathutils
    VERSION 1.0.0
    DESCRIPTION "A componentised C++ math utilities library"
    HOMEPAGE_URL "https://example.com/mathutils"
    LANGUAGES CXX)

# Standard installation directory layout (CMAKE_INSTALL_INCLUDEDIR, ...).
# Included early so the library's INSTALL_INTERFACE include paths resolve.
include(GNUInstallDirs)

# ---------------------------------------------------------------------------
# Project-wide settings (only enforced when mathutils is the top-level build,
# so that it stays a well-behaved dependency when added via add_subdirectory).
# ---------------------------------------------------------------------------
if(PROJECT_IS_TOP_LEVEL)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)

    if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
        set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    endif()
endif()

option(MATHUTILS_BUILD_DEMO "Build the mathutils demo executable" ON)
option(MATHUTILS_BUILD_TESTS "Build the mathutils unit tests" ON)

# ---------------------------------------------------------------------------
# Shared compiler-warning flags, exposed as an INTERFACE target that internal
# targets link against privately. Keeps warning policy in one place.
# ---------------------------------------------------------------------------
add_library(mathutils_warnings INTERFACE)
add_library(mathutils::warnings ALIAS mathutils_warnings)

if(MSVC)
    target_compile_options(mathutils_warnings INTERFACE /W4)
else()
    target_compile_options(mathutils_warnings INTERFACE
        -Wall -Wextra -Wpedantic -Wshadow -Wconversion)
endif()

# ---------------------------------------------------------------------------
# Components
# ---------------------------------------------------------------------------
add_subdirectory(src)

if(MATHUTILS_BUILD_DEMO)
    add_subdirectory(demo)
endif()

if(MATHUTILS_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ---------------------------------------------------------------------------
# Installation & CMake package config (find_package(mathutils) support)
# ---------------------------------------------------------------------------
if(PROJECT_IS_TOP_LEVEL)
    include(CMakePackageConfigHelpers)

    install(TARGETS mathutils mathutils_warnings
        EXPORT mathutilsTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

    # Public headers (skip the version.hpp.in template).
    install(DIRECTORY include/mathutils
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        PATTERN "*.in" EXCLUDE)

    # Generated version header.
    install(FILES ${PROJECT_BINARY_DIR}/generated/mathutils/version.hpp
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/mathutils)

    install(EXPORT mathutilsTargets
        FILE mathutilsTargets.cmake
        NAMESPACE mathutils::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mathutils)

    configure_package_config_file(
        cmake/mathutilsConfig.cmake.in
        ${PROJECT_BINARY_DIR}/mathutilsConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mathutils)

    write_basic_package_version_file(
        ${PROJECT_BINARY_DIR}/mathutilsConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion)

    install(FILES
        ${PROJECT_BINARY_DIR}/mathutilsConfig.cmake
        ${PROJECT_BINARY_DIR}/mathutilsConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mathutils)
endif()
