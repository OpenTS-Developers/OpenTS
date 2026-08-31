file(GLOB_RECURSE LANG_SRC CONFIGURE_DEPENDS
    "*.cpp" "*.c" "*.cc" "*.rc"
    "*.h" "*.hpp" "*.hm"
)

if(OPENTS_EXPERIMENTAL_CLANG_CL)
    set(CMAKE_DEPFILE_FLAGS_RC "")
    set(CMAKE_RC_COMPILE_OBJECT
        "<CMAKE_RC_COMPILER> /C 1252 <DEFINES> <INCLUDES> <FLAGS> /fo<OBJECT> <SOURCE>")
endif()

add_library(Language SHARED ${LANG_SRC})
target_compile_features(Language PRIVATE cxx_std_20)
target_compile_definitions(Language PRIVATE NOMINMAX)

# The version resource is stamped from the generated headers, which the resource compiler
# reaches through the target's include paths.
target_include_directories(Language PRIVATE "${OPENTS_GENERATED_DIR}")
add_dependencies(Language OpenTSBuildStamp)

if (OPENTS_MSVC_ABI_COMPILER)
    target_link_options(Language PRIVATE "/NOENTRY" "/NODEFAULTLIB")
endif()

add_custom_command(TARGET Language POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${TS_RUN_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:Language>"
        "${TS_RUN_DIR}/$<TARGET_FILE_NAME:Language>"
)
