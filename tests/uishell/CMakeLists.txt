# The harness links the vendored UI toolkits the way the engine does and loads the shipped
# documents through a recording render interface, so a runtime library mismatch, a document
# that fails to parse or a style outside the implemented render methods fails here. It lives
# outside code/ so that the recursive glob building OpenTS cannot pick this target's entry
# point up.
add_executable(UIShell
    "${CMAKE_CURRENT_SOURCE_DIR}/uishell.cpp"
    "${CMAKE_SOURCE_DIR}/code/ui/uidisplay.cpp"
    "${CMAKE_SOURCE_DIR}/code/ui/uigamectrl.cpp"
    "${CMAKE_SOURCE_DIR}/code/ui/uikeyboard.cpp"
    "${CMAKE_SOURCE_DIR}/code/ui/uikeys.cpp"
    "${CMAKE_SOURCE_DIR}/code/ui/uimainopt.cpp"
    "${CMAKE_SOURCE_DIR}/code/ui/uimsgbox.cpp"
    "${CMAKE_SOURCE_DIR}/code/ui/uirmlview.cpp"
    "${CMAKE_SOURCE_DIR}/code/ui/uiscreen.cpp"
    "${CMAKE_SOURCE_DIR}/code/ui/uisound.cpp"
    "${CMAKE_SOURCE_DIR}/code/ui/uiversion.cpp"
    "${CMAKE_SOURCE_DIR}/code/ui/uiwaitbox.cpp"
)

target_compile_features(UIShell PRIVATE cxx_std_20)

target_include_directories(UIShell PRIVATE "${CMAKE_SOURCE_DIR}/code" "${OPENTS_GENERATED_DIR}")

# The string-name table the shell resolves document references with is generated per build.
add_dependencies(UIShell OpenTSBuildStamp)

# The documents are read from the source tree, so the test needs no run directory.
target_compile_definitions(UIShell PRIVATE WIN32 _WINDOWS NOMINMAX "OPENTS_UI_DIR=\"${CMAKE_SOURCE_DIR}/ui\"")

target_compile_options(UIShell PRIVATE
    $<$<CONFIG:Debug>:/MTd /EHsc /Zc:__cplusplus>
    $<$<CONFIG:Release>:/MT /EHsc /Zc:__cplusplus>
)

target_link_libraries(UIShell PRIVATE RmlUi::Core freetype imgui kernel32 user32 shell32)

set_target_properties(UIShell PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_test(NAME uishell COMMAND UIShell)
