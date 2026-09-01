cmake_minimum_required(VERSION 3.23)

project(OpenTS VERSION 0.2.0 LANGUAGES C CXX ASM_MASM)

# A SemVer prerelease label, which project() cannot carry because it takes numbers only. Set it
# to "beta1" and the version reads 0.1.0-beta1; leave it empty outside a prerelease series. The
# manual's release registry is checked against both this and the project version.
set(OPENTS_VERSION_PRERELEASE "")

# Marks a build published under the version it declares, so that it is named by that version
# alone. Every other build names the commit it came from as well.
option(OPENTS_OFFICIAL_BUILD "Build as an official release of the declared version" OFF)

option(OPENTS_EXPERIMENTAL_CLANG_CL "Build with clang-cl using the MSVC ABI" OFF)

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
else()
    message(FATAL_ERROR
        "OpenTS requires the Visual Studio 2022 MSVC toolchain. "
        "For the unsupported clang-cl experiment, configure with "
        "-DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/clang-cl-msvc.cmake.")
endif()

enable_language(RC)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(TS_RUN_DIR "${CMAKE_SOURCE_DIR}/Run" CACHE PATH "Tiberian Sun run directory")

#
# ---------------------------------------------------------
# Build identity stamp
# ---------------------------------------------------------
#
# The version resources, the version displays, and the debug log's opening banner all report
# the version and commit that produced the binary. Stamping runs once here so the headers exist
# before the first compile, and again per build through OpenTSBuildStamp so that committing does
# not require a reconfigure to be reflected.
set(OPENTS_GENERATED_DIR "${CMAKE_BINARY_DIR}/generated")
set(OPENTS_VERSION_HEADER "${OPENTS_GENERATED_DIR}/opents_version.h")
set(OPENTS_STAMP_HEADER "${OPENTS_GENERATED_DIR}/opents_build.h")
file(MAKE_DIRECTORY "${OPENTS_GENERATED_DIR}")

set(OPENTS_STAMP_ARGS
    "-DOPENTS_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}"
    "-DOPENTS_VERSION_MINOR=${PROJECT_VERSION_MINOR}"
    "-DOPENTS_VERSION_PATCH=${PROJECT_VERSION_PATCH}"
    "-DOPENTS_VERSION_PRERELEASE=${OPENTS_VERSION_PRERELEASE}"
    "-DOPENTS_OFFICIAL_BUILD=${OPENTS_OFFICIAL_BUILD}"
    "-DOPENTS_SOURCE_DIR=${CMAKE_SOURCE_DIR}"
    "-DOPENTS_VERSION_HEADER=${OPENTS_VERSION_HEADER}"
    "-DOPENTS_STAMP_HEADER=${OPENTS_STAMP_HEADER}"
    -P "${CMAKE_SOURCE_DIR}/cmake/GitStamp.cmake"
)

execute_process(COMMAND ${CMAKE_COMMAND} ${OPENTS_STAMP_ARGS})

add_custom_target(OpenTSBuildStamp ALL
    COMMAND ${CMAKE_COMMAND} ${OPENTS_STAMP_ARGS}
    COMMENT "Stamping the build identity"
    VERBATIM
)

add_subdirectory(thirdparty)
add_subdirectory(code)
add_subdirectory(code/language)

enable_testing()
add_subdirectory(tests)
