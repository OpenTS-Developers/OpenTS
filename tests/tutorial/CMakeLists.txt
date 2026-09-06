# The tutorial text table is compiled straight into the harness along with the INI reader it
# takes its lines from. The entry point lives here so the recursive glob building OpenTS cannot
# pick it up.
add_executable(TutorialText
    "${CMAKE_CURRENT_SOURCE_DIR}/tutorialcontract.cpp"
    "${CMAKE_SOURCE_DIR}/code/tutorial.cpp"
    "${CMAKE_SOURCE_DIR}/code/ini.cpp"
    "${CMAKE_SOURCE_DIR}/code/dbgprint.cpp"
    "${CMAKE_SOURCE_DIR}/code/b64pipe.cpp"
    "${CMAKE_SOURCE_DIR}/code/b64straw.cpp"
    "${CMAKE_SOURCE_DIR}/code/base64.cpp"
    "${CMAKE_SOURCE_DIR}/code/buff.cpp"
    "${CMAKE_SOURCE_DIR}/code/crc.cpp"
    "${CMAKE_SOURCE_DIR}/code/cstraw.cpp"
    "${CMAKE_SOURCE_DIR}/code/int.cpp"
    "${CMAKE_SOURCE_DIR}/code/mpmath.cpp"
    "${CMAKE_SOURCE_DIR}/code/pipe.cpp"
    "${CMAKE_SOURCE_DIR}/code/pk.cpp"
    "${CMAKE_SOURCE_DIR}/code/straw.cpp"
    "${CMAKE_SOURCE_DIR}/code/trim.cpp"
    "${CMAKE_SOURCE_DIR}/code/xpipe.cpp"
    "${CMAKE_SOURCE_DIR}/code/xstraw.cpp"
)

target_compile_features(TutorialText PRIVATE cxx_std_20)

# The generated build stamp comes with dbgprint.cpp, so this target needs it too.
target_include_directories(TutorialText PRIVATE
    "${CMAKE_SOURCE_DIR}/code"
    "${OPENTS_GENERATED_DIR}"
)

add_dependencies(TutorialText OpenTSBuildStamp)

target_compile_definitions(TutorialText PRIVATE WIN32 _WINDOWS _MBCS NOMINMAX)

target_compile_options(TutorialText PRIVATE
    $<$<CONFIG:Debug>:/MTd /EHsc /Zc:__cplusplus>
    $<$<CONFIG:Release>:/MT /EHsc /Zc:__cplusplus>
)

target_link_libraries(TutorialText PRIVATE kernel32 user32 shell32)

set_target_properties(TutorialText PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_test(NAME tutorialtext COMMAND TutorialText)
