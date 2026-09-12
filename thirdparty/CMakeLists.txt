#
# ---------------------------------------------------------
# bgfx (rendering backend)
# ---------------------------------------------------------
#
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/bgfx.cmake/CMakeLists.txt")
    message(FATAL_ERROR
        "thirdparty/bgfx.cmake is empty. Fetch the submodules with:\n"
        "  git submodule update --init --recursive")
endif()

# bgfx builds shader and texture tools by default; the engine ships neither.
set(BGFX_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(BGFX_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BGFX_INSTALL OFF CACHE BOOL "" FORCE)
set(BGFX_CONFIG_RENDERER_WEBGPU OFF CACHE BOOL "" FORCE)

# The engine links the static CRT, so its dependencies must agree.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_subdirectory(bgfx.cmake)

# These libraries support the disabled texture tools and are not linked into OpenTS.
set_target_properties(bimg_decode bimg_encode PROPERTIES EXCLUDE_FROM_ALL TRUE)

if(OPENTS_EXPERIMENTAL_CLANG_CL AND CMAKE_SIZEOF_VOID_P EQUAL 4)
    # bx treats Clang with the MSVC CRT like a non-x86 compiler and erases
    # __stdcall. Restore it before Windows declarations are parsed.
    foreach(target bx bimg bgfx)
        target_compile_options(${target} PRIVATE
            "$<$<COMPILE_LANGUAGE:CXX>:/FI${PROJECT_SOURCE_DIR}/thirdparty/bx-clang-compat.h>"
        )
    endforeach()

    # The ASTC encoder selects a 64-bit popcount intrinsic when SSE4.2 is enabled,
    # but that intrinsic is unavailable for the Win32 target. Its SSE2 path is portable.
    target_compile_definitions(bimg PRIVATE ASTCENC_POPCNT=0 ASTCENC_SSE=20)
endif()

#
# ---------------------------------------------------------
# miniaudio (audio device, resampling, decoders)
# ---------------------------------------------------------
#
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/miniaudio/miniaudio.h")
    message(FATAL_ERROR
        "thirdparty/miniaudio is empty. Fetch the submodules with:\n"
        "  git submodule update --init --recursive")
endif()

# The library is built here rather than through its own CMakeLists.txt, which applies
# the MA_NO_* options privately; consumers of miniaudio.h must see the same set or the
# structures they share with the implementation change layout.
add_library(miniaudio STATIC "${CMAKE_CURRENT_SOURCE_DIR}/miniaudio-impl.c")
target_include_directories(miniaudio PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/miniaudio")

# Only the device layer, the resampler, and the decoders are used. The engine, node
# graph, and resource manager are left out along with generation and encoding.
target_compile_definitions(miniaudio PUBLIC
    MA_NO_ENGINE
    MA_NO_NODE_GRAPH
    MA_NO_RESOURCE_MANAGER
    MA_NO_GENERATION
    MA_NO_ENCODING
)

if(WIN32)
    # Backends are loaded at run time, so no import library is needed for them.
    target_compile_definitions(miniaudio PUBLIC
        MA_ENABLE_ONLY_SPECIFIC_BACKENDS
        MA_ENABLE_WASAPI
        MA_ENABLE_DSOUND
        MA_ENABLE_WINMM
        MA_ENABLE_NULL
    )
endif()

if(MSVC)
    # Match the engine's floating-point model so the resampler behaves the same in both.
    target_compile_options(miniaudio PRIVATE /arch:SSE2 /fp:precise)
endif()

#
# ---------------------------------------------------------
# LZO (LZO1X compression for saves, maps, and network blocks)
# ---------------------------------------------------------
#
# A vendored copy rather than a submodule; docs/BUILDING.md records why. It holds the three
# LZO1X-1 sources the engine calls and every header they reach.
#
# The library is built here rather than through its own CMakeLists.txt, which asks for
# compatibility with CMake 3.0 and is refused outright by CMake 4, and which also builds
# the test drivers and install rules that the engine does not ship.
file(GLOB LZO_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/lzo/src/*.c")
add_library(lzo STATIC ${LZO_SOURCES})

# LZO reads a generated config.h only when LZO_HAVE_CONFIG_H is defined, and probes the
# target with its own preprocessor tests otherwise. Leaving it undefined keeps the
# vendored tree free of a configure step.
target_include_directories(lzo
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/lzo/include"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/lzo/src"
)

#
# ---------------------------------------------------------
# FreeType (font rasterization for RmlUi)
# ---------------------------------------------------------
#
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/freetype/CMakeLists.txt")
    message(FATAL_ERROR
        "thirdparty/freetype is empty. Fetch the submodules with:\n"
        "  git submodule update --init --recursive")
endif()

# RmlUi defaults to shared libraries; the engine links every dependency statically.
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# Only TrueType rasterization is needed. The gzip module keeps FreeType's bundled zlib copy
# rather than a system one; every other optional decoder and shaper stays out.
set(FT_DISABLE_ZLIB ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BZIP2 ON CACHE BOOL "" FORCE)
set(FT_DISABLE_PNG ON CACHE BOOL "" FORCE)
set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BROTLI ON CACHE BOOL "" FORCE)
set(SKIP_INSTALL_ALL ON CACHE BOOL "" FORCE)

add_subdirectory(freetype)

# RmlUi looks for the target that FindFreetype would create. FreeType's own tree exports
# that name for installed consumers but never defines it inside a build tree.
add_library(Freetype::Freetype ALIAS freetype)

#
# ---------------------------------------------------------
# RmlUi (user interface documents)
# ---------------------------------------------------------
#
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/RmlUi/CMakeLists.txt")
    message(FATAL_ERROR
        "thirdparty/RmlUi is empty. Fetch the submodules with:\n"
        "  git submodule update --init --recursive")
endif()

set(RMLUI_FONT_ENGINE "freetype" CACHE STRING "" FORCE)
set(RMLUI_SAMPLES OFF CACHE BOOL "" FORCE)
set(RMLUI_LUA_BINDINGS OFF CACHE BOOL "" FORCE)
set(RMLUI_LOTTIE_PLUGIN OFF CACHE BOOL "" FORCE)
set(RMLUI_SVG_PLUGIN OFF CACHE BOOL "" FORCE)
set(RMLUI_TRACY_PROFILING OFF CACHE BOOL "" FORCE)

# The vendored FreeType above satisfies RmlUi's dependency check. A copy installed on the
# build machine must never be picked up in its place.
set(CMAKE_DISABLE_FIND_PACKAGE_Freetype ON)

add_subdirectory(RmlUi)

#
# ---------------------------------------------------------
# Dear ImGui (developer overlays)
# ---------------------------------------------------------
#
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.h")
    message(FATAL_ERROR
        "thirdparty/imgui is empty. Fetch the submodules with:\n"
        "  git submodule update --init --recursive")
endif()

# ImGui ships no build files. Only the core is compiled; the platform and renderer
# adapters are the engine's own.
add_library(imgui STATIC
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_demo.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_tables.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp"
)
target_include_directories(imgui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/imgui")

# Configuration macros change the layout of ImGui structures, so consumers must see the
# same set as the implementation.
target_compile_definitions(imgui PUBLIC IMGUI_DISABLE_OBSOLETE_FUNCTIONS)


#
# ---------------------------------------------------------
# Solution layout
# ---------------------------------------------------------
#

# Files a directory's targets, and those of every directory below it, under one solution
# folder. A target that already names a folder keeps it as a subfolder of that one, so the
# grouping bgfx gives its own targets survives.
function(opents_set_solution_folder directory folder)
    get_property(targets DIRECTORY "${directory}" PROPERTY BUILDSYSTEM_TARGETS)
    foreach(target IN LISTS targets)
        get_target_property(current ${target} FOLDER)
        if(current)
            set_target_properties(${target} PROPERTIES FOLDER "${folder}/${current}")
        else()
            set_target_properties(${target} PROPERTIES FOLDER "${folder}")
        endif()
    endforeach()

    get_property(subdirectories DIRECTORY "${directory}" PROPERTY SUBDIRECTORIES)
    foreach(subdirectory IN LISTS subdirectories)
        opents_set_solution_folder("${subdirectory}" "${folder}")
    endforeach()
endfunction()

opents_set_solution_folder("${CMAKE_CURRENT_SOURCE_DIR}" "Thirdparty")
