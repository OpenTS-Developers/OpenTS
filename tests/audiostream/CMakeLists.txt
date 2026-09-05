# The streams, the feeder and everything they use are compiled straight into the harness.
# It lives outside code/ so that the recursive glob building OpenTS cannot pick this
# target's entry point up.
add_executable(AudioStream
    "${CMAKE_CURRENT_SOURCE_DIR}/audiostream.cpp"
    "${CMAKE_SOURCE_DIR}/code/audio/audiostream.cpp"
    "${CMAKE_SOURCE_DIR}/code/audio/audiomixer.cpp"
    "${CMAKE_SOURCE_DIR}/code/audio/audiolevel.cpp"
    "${CMAKE_SOURCE_DIR}/code/audio/audiosample.cpp"
    "${CMAKE_SOURCE_DIR}/code/audio/audiodecode.cpp"
    "${CMAKE_SOURCE_DIR}/code/audio/audiodevice_null.cpp"
    "${CMAKE_SOURCE_DIR}/code/soscodec.cpp"
)

target_compile_features(AudioStream PRIVATE cxx_std_20)

target_include_directories(AudioStream PRIVATE
    "${CMAKE_SOURCE_DIR}/code"
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_compile_definitions(AudioStream PRIVATE WIN32 _WINDOWS _MBCS)

# The engine builds these sources with SSE2 and precise floating point. The harness matches
# that so a result here carries over to the engine.
target_compile_options(AudioStream PRIVATE
    $<$<CONFIG:Debug>:/MTd /EHsc /Zc:__cplusplus /arch:SSE2 /fp:precise>
    $<$<CONFIG:Release>:/MT /EHsc /Zc:__cplusplus /arch:SSE2 /fp:precise>
)

target_link_libraries(AudioStream PRIVATE miniaudio kernel32 user32 shell32)

set_target_properties(AudioStream PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_test(NAME audiostream COMMAND AudioStream)
