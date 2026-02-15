# -----------------------------------------------------------------------------
# Determine CMake version and build type.
# -----------------------------------------------------------------------------
# NOTE: cmake_minimum_required() and project() *MUST* be the two first commands
# used, see https://cmake.org/cmake/help/v3.3/command/project.html -- the
# latter in particular handles loading a bunch of shared CMake definitions
# and loading the cross-compilation settings from CMAKE_TOOLCHAIN_FILE.

cmake_minimum_required(VERSION 3.16)
project(WebKit)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "AmigaOS")    
    # Should this Qt be static or dynamically linked?
    option(BUILD_SHARED_LIBS "Build Qt statically or dynamically" ON)
    set(QT_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})
else()
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "Force static build" FORCE)
    set(QT_BUILD_SHARED_LIBS OFF CACHE BOOL "Force static build" FORCE)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    set(CMAKE_SHARED_LIBRARY_SUFFIX ".a")
    set(CMAKE_EXTRA_SHARED_LIBRARY_SUFFIXES "")
endif()

if(AMIGA)
    set(CMAKE_DL_LIBS "")
    set(WTF_CPU_PPC 1)
    set(WTF_CPU_KNOWN 1)
endif()

# Remove this cmake_policy() after upgrading cmake_minimum_required() to 3.20.
if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.20")
    cmake_policy(SET CMP0116 OLD)
endif ()

# -----------------------------------------------------------------------------
# Common configuration
#------------------------------------------------------------------------------
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/Source/cmake")
include(WebKitCommon)

# -----------------------------------------------------------------------------
# Enable API unit tests and create a target for the test runner
# -----------------------------------------------------------------------------
if (ENABLE_API_TESTS)
    enable_testing()

    # Qt API tests are at top level because of Coin requirements
    if (${PORT} STREQUAL "Qt")
        add_subdirectory(tests)
    endif ()
endif ()

# -----------------------------------------------------------------------------
# Add module directories
# -----------------------------------------------------------------------------
add_subdirectory(Source)

# -----------------------------------------------------------------------------
# Add tools
# -----------------------------------------------------------------------------
if (ENABLE_TOOLS)
    add_subdirectory(Tools)
endif ()

if (DEVELOPER_MODE)
    add_subdirectory(PerformanceTests)
endif ()

# -----------------------------------------------------------------------------
# Print the features list last, for maximum visibility.
# -----------------------------------------------------------------------------
PRINT_WEBKIT_OPTIONS()
