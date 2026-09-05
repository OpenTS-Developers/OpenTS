# The rings are header-only; the harness needs nothing from code/ beyond the include path.
add_executable(AudioRing
    "${CMAKE_CURRENT_SOURCE_DIR}/audioring.cpp"
)

target_compile_features(AudioRing PRIVATE cxx_std_20)

target_include_directories(AudioRing PRIVATE
    "${CMAKE_SOURCE_DIR}/code"
)

target_compile_definitions(AudioRing PRIVATE WIN32 _WINDOWS _MBCS)

target_compile_options(AudioRing PRIVATE
    $<$<CONFIG:Debug>:/MTd /EHsc /Zc:__cplusplus /arch:SSE2 /fp:precise>
    $<$<CONFIG:Release>:/MT /EHsc /Zc:__cplusplus /arch:SSE2 /fp:precise>
)

target_link_libraries(AudioRing PRIVATE kernel32 user32 shell32)

set_target_properties(AudioRing PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_test(NAME audioring COMMAND AudioRing)
