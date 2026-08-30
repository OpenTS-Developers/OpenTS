# Limit the generated solution to supported configurations.
set(CMAKE_CONFIGURATION_TYPES Debug Release CACHE STRING "" FORCE)

# OpenTS currently supports 32-bit (x86) builds only.
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "OpenTS must be built as 32-bit x86. Reconfigure with -A Win32.")
endif()

#
# ---------------------------------------------------------
# Directories that build as their own project
# ---------------------------------------------------------
#
set(SUBPROJECT_DIRS
    "language"
    "vqalib"
)

#
# ---------------------------------------------------------
# Gather all potential OpenTS files
# ---------------------------------------------------------
#
file(GLOB_RECURSE OPENTS_RAW CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hh"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.rc"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.asm"
)

#
# ---------------------------------------------------------
# Remove files that belong to a subproject
# ---------------------------------------------------------
#
set(OPENTS_SRC "")
foreach(f ${OPENTS_RAW})
    set(EXCLUDED FALSE)
    foreach(dir ${SUBPROJECT_DIRS})
        if(f MATCHES "^${CMAKE_CURRENT_SOURCE_DIR}/${dir}/.*")
            set(EXCLUDED TRUE)
        endif()
    endforeach()

    if(NOT EXCLUDED)
        list(APPEND OPENTS_SRC "${f}")
    endif()
endforeach()

#
# ---------------------------------------------------------
# Create actual OpenTS executable (Win32)
# ---------------------------------------------------------
#
message(STATUS "${PROJECT_NAME}: Creating '${PROJECT_NAME}' executable...")

add_executable(OpenTS WIN32 ${OPENTS_SRC})
target_compile_features(OpenTS PRIVATE cxx_std_20)

#
# ---------------------------------------------------------
# Output paths
# ---------------------------------------------------------
#
message(STATUS "${PROJECT_NAME}: Setting binary output paths...")

set_target_properties(OpenTS PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"

    # Built under the shipped names, so the executable, the symbol file the debug record
    # names, and the map reach the run directory without a rename.
    OUTPUT_NAME_DEBUG "GameD"
    OUTPUT_NAME_RELEASE "Game"
    PDB_NAME_DEBUG "GameD"
    PDB_NAME_RELEASE "Game"
)

set_target_properties(OpenTS PROPERTIES
    VS_DEBUGGER_COMMAND "${TS_RUN_DIR}/$<TARGET_FILE_NAME:OpenTS>"
    VS_DEBUGGER_WORKING_DIRECTORY "${TS_RUN_DIR}"
)

#
# ---------------------------------------------------------
# Compile flags
# ---------------------------------------------------------
#
message(STATUS "${PROJECT_NAME}: Applying compiler flags...")

set(OPENTS_COMPILE_OPTIONS

    # ================= DEBUG =================
    $<$<CONFIG:Debug>:

        # ---------- C++ ----------
        # /arch:SSE2 -- IEEE-754 single precision, no x87 excess precision.
        # /fp:precise -- no reassociation of the engine's accumulations.
        $<$<COMPILE_LANGUAGE:CXX>:
            /Zi /Od /RTC1 /GR /MP /EHsc /Oy- /MTd /Zc:__cplusplus /arch:SSE2 /fp:precise
        >

        # ---------- C ----------
        $<$<COMPILE_LANGUAGE:C>:
            /Zi /Od /RTC1 /MP /Oy- /arch:SSE2 /fp:precise
        >

        # ---------- MASM ----------
        $<$<COMPILE_LANGUAGE:ASM_MASM>:
            /Zi /c /coff /Cx /safeseh
        >
    >

    # ================= RELEASE =================
    $<$<CONFIG:Release>:

        # ---------- C++ ----------
        # /arch:SSE2 -- IEEE-754 single precision, no x87 excess precision.
        # /fp:precise -- no reassociation of the engine's accumulations.
        $<$<COMPILE_LANGUAGE:CXX>:
            /Zi /O2 /GF /GR /MP /EHsc /MT /Zc:__cplusplus /arch:SSE2 /fp:precise
        >

        # ---------- C ----------
        $<$<COMPILE_LANGUAGE:C>:
            /Zi /O2 /GF /MP /arch:SSE2 /fp:precise
        >

        # ---------- MASM ----------
        $<$<COMPILE_LANGUAGE:ASM_MASM>:
            /Zi /c /coff /Cx /safeseh
        >
    >
)

target_compile_options(OpenTS PRIVATE ${OPENTS_COMPILE_OPTIONS})

#
# ---------------------------------------------------------
# Subprojects
# ---------------------------------------------------------
#
# The VQA player carries its own project, and hands OpenTS its include path along with the
# library, so the movie code is reached the same way from anywhere in the solution.
add_subdirectory(vqalib)

message(STATUS "${PROJECT_NAME}: Adding include paths...")
target_include_directories(OpenTS PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${OPENTS_GENERATED_DIR}"
)

# The generated build stamp has to exist before anything compiles.
add_dependencies(OpenTS OpenTSBuildStamp)

# Only the renderer's own translation unit sees bgfx, so its headers and the shaders it
# embeds are put on that file rather than on the whole target.
set(BGFX_ROOT "${CMAKE_SOURCE_DIR}/thirdparty/bgfx.cmake/bgfx")
set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/bgfxbackend.cpp" PROPERTIES
    INCLUDE_DIRECTORIES
        "${BGFX_ROOT}/include;${CMAKE_SOURCE_DIR}/thirdparty/bgfx.cmake/bx/include;${BGFX_ROOT}/examples/common/imgui"
    COMPILE_OPTIONS "/Zc:preprocessor"
)

message(STATUS "${PROJECT_NAME}: Adding compilier definitions...")
target_compile_definitions(OpenTS PRIVATE
    WIN32
    _WINDOWS
    _MBCS

    # Compiles Blowfish into the binary instead of reaching it through the COM object in
    # blowfish.dll. It decides the layout of BlowfishEngine, so every translation unit has
    # to agree on it and it belongs on the compile line rather than in a header.
    NO_BLOWFISH_DLL
)

#
# ---------------------------------------------------------
# Windows libraries and linker options
# ---------------------------------------------------------
#
message(STATUS "${PROJECT_NAME}: Applying linker options...")

# Required libraries:
#  Comctl32.lib
#  DbgHelp.lib
#  DSound.lib
#  Version.lib
#  Winmm.lib
#  wsock32.lib
#  + all Win32 libs
#

# The bgfx targets are linked by file rather than by name. Their interface settings
# include compiler switches and a directory of headers that shadow the system ones, and
# the Visual Studio generator would hand all of that to ML.EXE as well, which fails.
add_dependencies(OpenTS bgfx bx bimg)
target_link_libraries(OpenTS PRIVATE
    VQALib

    $<TARGET_FILE:bgfx>
    $<TARGET_FILE:bx>
    $<TARGET_FILE:bimg>
    comctl32
    dbghelp
    dsound
    iphlpapi
    psapi
    version
    winmm
    wsock32
    kernel32 user32 gdi32 winspool comdlg32 advapi32 shell32
    ole32 oleaut32 uuid odbc32 odbccp32
)

if (MSVC)
    target_link_options(OpenTS PRIVATE
        /SUBSYSTEM:WINDOWS
        /DEBUG
        /MAP
        # Raises the user address space from 2GB to 4GB under 64-bit Windows. Pointers may
        # then have the high bit set, so anything holding one in a signed integer must only
        # round-trip the bits and never compare or scale them.
        /LARGEADDRESSAWARE
        # Declares the process per-monitor DPI aware, so a scaled desktop does not blur
        # the frame by stretching the whole window.
        "/MANIFESTINPUT:${CMAKE_CURRENT_SOURCE_DIR}/sun.manifest"
        /MANIFEST:EMBED
    )
endif()

#
# ---------------------------------------------------------
# MASM assembly support
# ---------------------------------------------------------
#
message(STATUS "${PROJECT_NAME}: Adding MASM support...")

enable_language(ASM_MASM)

# All .asm files compile automatically using ML.EXE with appropriate flags.
foreach(f ${OPENTS_SRC})
    if(f MATCHES "\\.asm$")
        set_source_files_properties(${f} PROPERTIES
            LANGUAGE ASM_MASM
            COMPILE_FLAGS "-Zi -c -coff -Cx"
        )
    endif()
endforeach()

#
# ---------------------------------------------------------
# Visual Studio source groups
# ---------------------------------------------------------
#
message(STATUS "${PROJECT_NAME}: Setting up Visual Studio filters...")

# Init Source Files (_*.cpp)
foreach(f ${OPENTS_SRC})
    if(f MATCHES "[/\\\\]_.*\\.cpp$")
        source_group("Source Files\\Init Source Files" FILES ${f})
    endif()
endforeach()

# Init Header Files (_*.h)
foreach(f ${OPENTS_SRC})
    if(f MATCHES "[/\\\\]_.*\\.h$")
        source_group("Header Files\\Init Header Files" FILES ${f})
    endif()
endforeach()

# Define Header Files (*.hh)
foreach(f ${OPENTS_SRC})
    if(f MATCHES "\\.hh$")
        source_group("Header Files\\Define Header Files" FILES ${f})
    endif()
endforeach()

# Assembly Files
foreach(f ${OPENTS_SRC})
    if(f MATCHES "\\.asm$")
        source_group("Assembly Files" FILES ${f})
    endif()
endforeach()

# Resource Files
foreach(f ${OPENTS_SRC})
    if(f MATCHES "\\.(ico|cur|bmp|gif|jpe?g|rc)$")
        source_group("Resource Files" FILES ${f})
    endif()
endforeach()

# List of all interface filenames (headers + C stubs)
set(INTERFACE_FILES
    iblockci.h       iblockci_i.c
    iblowfish.h      iblowfish_i.c
    iflyctrl.h       iflyctrl_i.c
    ilinkstm.h
    iloco.h          iloco_i.c
    ilocos.h         ilocos_i.c
    ipiggy.h         ipiggy_i.c
    isun.h           isun_i.c
)

# Convert to full paths relative to source dir
set(FULL_INTERFACE_FILES "")
foreach(f IN LISTS INTERFACE_FILES)
    list(APPEND FULL_INTERFACE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${f}")
endforeach()

# Add them to the "Interface Files" group and exclude *_i.c from build
foreach(f IN LISTS FULL_INTERFACE_FILES)

    # Put into VS filter
    source_group("Interface Files" FILES "${f}")

    # Exclude *_i.c from build, but DO NOT mark headers as header-only
    if(f MATCHES "_i\\.c$")
        set_source_files_properties("${f}" PROPERTIES HEADER_FILE_ONLY TRUE)
    endif()

endforeach()

# General source files
source_group("Source Files" REGULAR_EXPRESSION ".*\\.(c|cpp)$")
source_group("Header Files" REGULAR_EXPRESSION ".*\\.(h|hpp)$")

#
# ---------------------------------------------------------
# Populate the local run directory
# ---------------------------------------------------------
#
message(STATUS "${PROJECT_NAME}: Creating post-build commands...")
add_custom_command(TARGET OpenTS POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "${TS_RUN_DIR}"
)

add_custom_command(TARGET OpenTS POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:OpenTS>"
        "${TS_RUN_DIR}"
)

# Copy the linker-generated .pdb alongside the exe
add_custom_command(TARGET OpenTS POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_PDB_FILE:OpenTS>"
        "${TS_RUN_DIR}"
)

# Copy the linker-generated .map (sits next to the exe, no GenEx for it)
add_custom_command(TARGET OpenTS POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE_DIR:OpenTS>/$<TARGET_FILE_BASE_NAME:OpenTS>.map"
        "${TS_RUN_DIR}"
)

#
# ---------------------------------------------------------
# Visual Studio startup project
# ---------------------------------------------------------
#
set_property(GLOBAL PROPERTY VS_STARTUP_PROJECT OpenTS)

# ---------------------------------------------------------
# Add the .natvis file for Visual Studio debugging
# ---------------------------------------------------------
set(NATVIS_FILE "${CMAKE_CURRENT_SOURCE_DIR}/sun.natvis")
source_group("Natvis Files" FILES ${NATVIS_FILE})
target_sources(OpenTS PRIVATE ${NATVIS_FILE})
target_link_options(OpenTS PRIVATE "/NATVIS:${NATVIS_FILE}")
