file(GLOB_RECURSE LANG_SRC CONFIGURE_DEPENDS
    "*.cpp" "*.c" "*.cc" "*.rc"
    "*.h" "*.hpp" "*.hm"
)

# Resource scripts need the Windows resource compiler.
if(NOT WIN32)
    list(FILTER LANG_SRC EXCLUDE REGEX "\\.rc$")
endif()

add_library(Language SHARED ${LANG_SRC})
target_compile_features(Language PRIVATE cxx_std_20)
target_compile_definitions(Language PRIVATE NOMINMAX)

# The version resource is stamped from the generated headers, which the resource compiler
# reaches through the target's include paths.
target_include_directories(Language PRIVATE "${OPENTS_GENERATED_DIR}")
add_dependencies(Language OpenTSBuildStamp)

# The game loads its strings from this library at runtime, so building the executable alone
# would leave a stale Language.dll beside it.
add_dependencies(OpenTS Language)

if(MSVC)
    target_link_options(Language PRIVATE "/NOENTRY" "/NODEFAULTLIB")
endif()

# The game loads this library by name from its own directory, so it is built beside the
# executable rather than copied there.
set_target_properties(Language PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
