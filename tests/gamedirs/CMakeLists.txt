# The directory handling is compiled straight into the harness, along with the file classes
# and the INI reader it stands on. It lives outside code/ so that the recursive glob building
# OpenTS cannot pick this target's entry point up.
add_executable(GameDirs
    "${CMAKE_CURRENT_SOURCE_DIR}/gamedirscontract.cpp"
    "${CMAKE_SOURCE_DIR}/code/gamedirs.cpp"
    "${CMAKE_SOURCE_DIR}/code/cdfile.cpp"
    "${CMAKE_SOURCE_DIR}/code/bfiofile.cpp"
    "${CMAKE_SOURCE_DIR}/code/rawfile.cpp"
    "${CMAKE_SOURCE_DIR}/code/dbgprint.cpp"
    "${CMAKE_SOURCE_DIR}/code/ini.cpp"
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

target_compile_features(GameDirs PRIVATE cxx_std_20)

# The generated build stamp comes with dbgprint.cpp, so this target needs it too.
target_include_directories(GameDirs PRIVATE
    "${CMAKE_SOURCE_DIR}/code"
    "${OPENTS_GENERATED_DIR}"
)

add_dependencies(GameDirs OpenTSBuildStamp)

target_compile_definitions(GameDirs PRIVATE WIN32 _WINDOWS _MBCS NOMINMAX)

target_compile_options(GameDirs PRIVATE
    $<$<CONFIG:Debug>:/MTd /EHsc /Zc:__cplusplus>
    $<$<CONFIG:Release>:/MT /EHsc /Zc:__cplusplus>
)

target_link_libraries(GameDirs PRIVATE base64 kernel32 user32 shell32)

set_target_properties(GameDirs PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_test(NAME gamedirs COMMAND GameDirs)
