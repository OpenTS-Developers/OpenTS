# The cache and the decoders it uses are compiled straight into the harness. It lives
# outside code/ so that the recursive glob building OpenTS cannot pick this target's entry
# point up.
add_executable(AudioCache
    "${CMAKE_CURRENT_SOURCE_DIR}/audiocache.cpp"
    "${CMAKE_SOURCE_DIR}/code/audio/audiosample.cpp"
    "${CMAKE_SOURCE_DIR}/code/audio/audiodecode.cpp"
    "${CMAKE_SOURCE_DIR}/code/soscodec.cpp"
)

target_compile_features(AudioCache PRIVATE cxx_std_20)

target_include_directories(AudioCache PRIVATE
    "${CMAKE_SOURCE_DIR}/code"
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_compile_definitions(AudioCache PRIVATE WIN32 _WINDOWS _MBCS)

target_compile_options(AudioCache PRIVATE
    $<$<CONFIG:Debug>:/MTd /EHsc /Zc:__cplusplus /arch:SSE2 /fp:precise>
    $<$<CONFIG:Release>:/MT /EHsc /Zc:__cplusplus /arch:SSE2 /fp:precise>
)

target_link_libraries(AudioCache PRIVATE miniaudio kernel32 user32 shell32)

set_target_properties(AudioCache PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_test(NAME audiocache COMMAND AudioCache)
