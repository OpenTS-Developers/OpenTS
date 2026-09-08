# The harness links the vendored UI toolkits the way the engine will, so a runtime library
# or structure layout mismatch fails here before any screen exists. It lives outside code/
# so that the recursive glob building OpenTS cannot pick this target's entry point up.
add_executable(UIShell
    "${CMAKE_CURRENT_SOURCE_DIR}/uishell.cpp"
)

target_compile_features(UIShell PRIVATE cxx_std_20)

target_compile_definitions(UIShell PRIVATE WIN32 _WINDOWS NOMINMAX)

target_compile_options(UIShell PRIVATE
    $<$<CONFIG:Debug>:/MTd /EHsc /Zc:__cplusplus>
    $<$<CONFIG:Release>:/MT /EHsc /Zc:__cplusplus>
)

target_link_libraries(UIShell PRIVATE RmlUi::Core freetype imgui kernel32 user32 shell32)

set_target_properties(UIShell PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_test(NAME uishell COMMAND UIShell)
