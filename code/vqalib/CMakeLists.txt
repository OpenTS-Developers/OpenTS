#
# ---------------------------------------------------------
# Gather the VQALib player files
# ---------------------------------------------------------
#
file(GLOB VQALIB_SRC CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

#
# ---------------------------------------------------------
# Create the VQALib player library
# ---------------------------------------------------------
#
message(STATUS "${PROJECT_NAME}: Creating 'VQALib' library...")

add_library(VQALib STATIC ${VQALIB_SRC})
target_compile_features(VQALib PRIVATE cxx_std_20)

set_target_properties(VQALib PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

# The player and the engine have to be built the same way, since the objects end up in one
# binary and have to agree on the runtime library and the floating-point model.
target_compile_options(VQALib PRIVATE ${OPENTS_COMPILE_OPTIONS})

target_compile_definitions(VQALib PRIVATE
    WIN32
    _WINDOWS
    _MBCS
    NOMINMAX
)

# Callers reach the vqaplay.h family through the library, while the player itself reaches
# back into the engine for ahandle.h, which owns the sound handle it plays through.
target_include_directories(VQALib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(VQALib PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/..")

#
# ---------------------------------------------------------
# Visual Studio filters
# ---------------------------------------------------------
#
foreach(f ${VQALIB_SRC})
    if(f MATCHES "\\.(c|cpp)$")
        source_group("Source Files" FILES ${f})
    elseif(f MATCHES "\\.h$")
        source_group("Header Files" FILES ${f})
    endif()
endforeach()
