# The launch file reader is compiled straight into the harness along with the INI reader it
# is written against. It lives outside code/ so that the recursive glob building the engine
# cannot pick this target's entry point up.
add_executable(SpawnContract
    "${CMAKE_CURRENT_SOURCE_DIR}/spawncontract.cpp"
    "${CMAKE_SOURCE_DIR}/code/spawnerconfig.cpp"
    "${CMAKE_SOURCE_DIR}/code/ini.cpp"
    "${CMAKE_SOURCE_DIR}/code/dbgprint.cpp"
    "${CMAKE_SOURCE_DIR}/code/trim.cpp"
    "${CMAKE_SOURCE_DIR}/code/buff.cpp"
    "${CMAKE_SOURCE_DIR}/code/straw.cpp"
    "${CMAKE_SOURCE_DIR}/code/xstraw.cpp"
    "${CMAKE_SOURCE_DIR}/code/cstraw.cpp"
    "${CMAKE_SOURCE_DIR}/code/pipe.cpp"
    "${CMAKE_SOURCE_DIR}/code/xpipe.cpp"
    "${CMAKE_SOURCE_DIR}/code/b64straw.cpp"
    "${CMAKE_SOURCE_DIR}/code/b64pipe.cpp"
    "${CMAKE_SOURCE_DIR}/code/base64.cpp"
    "${CMAKE_SOURCE_DIR}/code/crc.cpp"
    "${CMAKE_SOURCE_DIR}/code/pk.cpp"
    "${CMAKE_SOURCE_DIR}/code/int.cpp"
    "${CMAKE_SOURCE_DIR}/code/mpmath.cpp"
)

target_compile_features(SpawnContract PRIVATE cxx_std_20)

# The generated build stamp comes with dbgprint.cpp, so this target needs it too.
target_include_directories(SpawnContract PRIVATE
    "${CMAKE_SOURCE_DIR}/code"
    "${OPENTS_GENERATED_DIR}"
)

add_dependencies(SpawnContract OpenTSBuildStamp)

target_compile_definitions(SpawnContract PRIVATE WIN32 _WINDOWS _MBCS NOMINMAX)

target_compile_options(SpawnContract PRIVATE
    $<$<CONFIG:Debug>:/MTd /EHsc /Zc:__cplusplus>
    $<$<CONFIG:Release>:/MT /EHsc /Zc:__cplusplus>
)

target_link_libraries(SpawnContract PRIVATE kernel32 user32 shell32)

set_target_properties(SpawnContract PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_test(NAME spawncontract COMMAND SpawnContract)
