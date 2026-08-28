# The decoders are compiled straight into the harness. It lives outside code/ so that the
# recursive glob building OpenTS cannot pick this target's entry point up.
add_executable(UnvqDec
    "${CMAKE_CURRENT_SOURCE_DIR}/unvqparity.cpp"
    "${CMAKE_SOURCE_DIR}/code/unvqdec.cpp"
)

target_compile_features(UnvqDec PRIVATE cxx_std_20)

target_include_directories(UnvqDec PRIVATE
    "${CMAKE_SOURCE_DIR}/code"
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_compile_definitions(UnvqDec PRIVATE WIN32 _WINDOWS _MBCS)

# The engine builds this source with SSE2 and precise floating point. The harness matches that
# so a result here carries over to the engine.
target_compile_options(UnvqDec PRIVATE
    $<$<CONFIG:Debug>:/MTd /EHsc /Zc:__cplusplus /arch:SSE2 /fp:precise>
    $<$<CONFIG:Release>:/MT /EHsc /Zc:__cplusplus /arch:SSE2 /fp:precise>
)

target_link_libraries(UnvqDec PRIVATE kernel32 user32 shell32)

set_target_properties(UnvqDec PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_test(NAME unvqdec COMMAND UnvqDec)
