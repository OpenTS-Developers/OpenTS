cmake_minimum_required(VERSION 3.23)

project(OpenTS VERSION 0.3.0 LANGUAGES C CXX)

# The SemVer prerelease label, set apart because project() accepts only numbers. With "beta1",
# version 1.2.3 reads 1.2.3-beta1. manual/data/releases.yaml must match this and the project
# version.
set(OPENTS_VERSION_PRERELEASE "")

# A release candidate's label, such as "rc1", normally taken from a v<version>-<label> tag at
# HEAD. The release workflow sets it from the tag it builds.
set(OPENTS_VERSION_LABEL "" CACHE STRING "Prerelease label to build with, such as rc1")

# An official build displays its version alone. Any other build also displays its commit when
# git reports one.
option(OPENTS_OFFICIAL_BUILD "Build as an official release of the declared version" OFF)

option(OPENTS_EXPERIMENTAL_CLANG_CL "Build with clang-cl using the MSVC ABI" OFF)

# Registers the tests that take over the desktop: they raise their own window, set the keyboard
# state and move the pointer, and fail while another window holds the focus.
option(OPENTS_DESKTOP_TESTS "Register the tests that need an otherwise idle desktop" OFF)

# Lets a target's FOLDER place it in a Visual Studio solution folder. The bgfx submodule sets
# this as well, so declare it here rather than inherit it from a vendored build.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)


if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang"
   AND CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    if(NOT OPENTS_EXPERIMENTAL_CLANG_CL)
        message(FATAL_ERROR
            "The unsupported clang-cl experiment must be configured with "
            "-DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/clang-cl-msvc.cmake.")
    endif()
elseif(MSVC)
    if(MSVC_VERSION LESS 1930)
        message(FATAL_ERROR "OpenTS requires MSVC 19.30 or newer.")
    endif()
    if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4 AND NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
        message(FATAL_ERROR
            "OpenTS targets Win32 and x64. Reconfigure with -A Win32 or -A x64.")
    endif()
else()
    message(FATAL_ERROR
        "OpenTS requires the Visual Studio 2022 MSVC toolchain. "
        "For the unsupported clang-cl experiment, configure with "
        "-DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/clang-cl-msvc.cmake.")
endif()

string(TOLOWER "${CMAKE_CXX_COMPILER_ARCHITECTURE_ID}" OPENTS_ARCH)
if(NOT OPENTS_ARCH)
    set(OPENTS_ARCH "unknown")
endif()

enable_language(RC)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(OPENTS_GAME_DIR "${CMAKE_SOURCE_DIR}/Run" CACHE PATH "Tiberian Sun game data directory")

#
# ---------------------------------------------------------
# Build identity stamp
# ---------------------------------------------------------
#
# cmake/GitStamp.cmake runs here so the headers exist before the first compile, and on every
# build through OpenTSBuildStamp so a new commit needs no reconfigure. The UI string-name table
# is generated from language.h on the same schedule.
set(OPENTS_GENERATED_DIR "${CMAKE_BINARY_DIR}/generated")
set(OPENTS_VERSION_HEADER "${OPENTS_GENERATED_DIR}/opents_version.h")
set(OPENTS_STAMP_HEADER "${OPENTS_GENERATED_DIR}/opents_build.h")
set(OPENTS_STRINGS_HEADER "${OPENTS_GENERATED_DIR}/opents_strings.h")
file(MAKE_DIRECTORY "${OPENTS_GENERATED_DIR}")

set(OPENTS_STAMP_ARGS
    "-DOPENTS_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}"
    "-DOPENTS_VERSION_MINOR=${PROJECT_VERSION_MINOR}"
    "-DOPENTS_VERSION_PATCH=${PROJECT_VERSION_PATCH}"
    "-DOPENTS_VERSION_PRERELEASE=${OPENTS_VERSION_PRERELEASE}"
    "-DOPENTS_VERSION_LABEL=${OPENTS_VERSION_LABEL}"
    "-DOPENTS_OFFICIAL_BUILD=${OPENTS_OFFICIAL_BUILD}"
    "-DOPENTS_SOURCE_DIR=${CMAKE_SOURCE_DIR}"
    "-DOPENTS_VERSION_HEADER=${OPENTS_VERSION_HEADER}"
    "-DOPENTS_STAMP_HEADER=${OPENTS_STAMP_HEADER}"
    "-DOPENTS_ARCH=${OPENTS_ARCH}"
    -P "${CMAKE_SOURCE_DIR}/cmake/GitStamp.cmake"
)

set(OPENTS_STRINGS_ARGS
    "-DOPENTS_LANGUAGE_HEADER=${CMAKE_SOURCE_DIR}/code/language/language.h"
    "-DOPENTS_STRINGS_HEADER=${OPENTS_STRINGS_HEADER}"
    -P "${CMAKE_SOURCE_DIR}/cmake/StringTable.cmake"
)

execute_process(COMMAND ${CMAKE_COMMAND} ${OPENTS_STAMP_ARGS})
execute_process(COMMAND ${CMAKE_COMMAND} ${OPENTS_STRINGS_ARGS})

add_custom_target(OpenTSBuildStamp ALL
    COMMAND ${CMAKE_COMMAND} ${OPENTS_STAMP_ARGS}
    COMMAND ${CMAKE_COMMAND} ${OPENTS_STRINGS_ARGS}
    COMMENT "Stamping the build identity and the string-name table"
    VERBATIM
)

add_subdirectory(thirdparty)
add_subdirectory(code)
add_subdirectory(code/language)

enable_testing()
add_subdirectory(tests)
