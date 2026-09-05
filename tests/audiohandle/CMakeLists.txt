# The handle encoding is header-only; the harness needs nothing from code/ beyond the
# include path.
add_executable(AudioHandle
    "${CMAKE_CURRENT_SOURCE_DIR}/audiohandle.cpp"
)

target_compile_features(AudioHandle PRIVATE cxx_std_20)

target_include_directories(AudioHandle PRIVATE
    "${CMAKE_SOURCE_DIR}/code"
)

target_compile_definitions(AudioHandle PRIVATE WIN32 _WINDOWS _MBCS)

target_compile_options(AudioHandle PRIVATE
    $<$<CONFIG:Debug>:/MTd /EHsc /Zc:__cplusplus /arch:SSE2 /fp:precise>
    $<$<CONFIG:Release>:/MT /EHsc /Zc:__cplusplus /arch:SSE2 /fp:precise>
)

target_link_libraries(AudioHandle PRIVATE kernel32 user32 shell32)

set_target_properties(AudioHandle PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_test(NAME audiohandle COMMAND AudioHandle)
