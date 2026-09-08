# The LZO block classes are compiled straight into the harness against the same vendored
# library the engine links. It lives outside code/ so that the recursive glob building OpenTS
# cannot pick this target's entry point up.
add_executable(LzoBlock
    "${CMAKE_CURRENT_SOURCE_DIR}/lzoblock.cpp"
    "${CMAKE_SOURCE_DIR}/code/buff.cpp"
    "${CMAKE_SOURCE_DIR}/code/lzopipe.cpp"
    "${CMAKE_SOURCE_DIR}/code/lzostraw.cpp"
    "${CMAKE_SOURCE_DIR}/code/pipe.cpp"
    "${CMAKE_SOURCE_DIR}/code/straw.cpp"
    "${CMAKE_SOURCE_DIR}/code/xpipe.cpp"
    "${CMAKE_SOURCE_DIR}/code/xstraw.cpp"
)

target_compile_features(LzoBlock PRIVATE cxx_std_20)

target_include_directories(LzoBlock PRIVATE
    "${CMAKE_SOURCE_DIR}/code"
)

target_compile_definitions(LzoBlock PRIVATE WIN32 _WINDOWS _MBCS NOMINMAX)

# The engine builds these sources with SSE2 and precise floating point. The harness matches
# that so a result here carries over to the engine.
target_compile_options(LzoBlock PRIVATE
    $<$<CONFIG:Debug>:/MTd /EHsc /Zc:__cplusplus /arch:SSE2 /fp:precise>
    $<$<CONFIG:Release>:/MT /EHsc /Zc:__cplusplus /arch:SSE2 /fp:precise>
)

target_link_libraries(LzoBlock PRIVATE lzo kernel32 user32 shell32)

set_target_properties(LzoBlock PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_test(NAME lzoblock COMMAND LzoBlock)
