# Links the macOS command-line and key-name shims. The argument carries an accent so a
# byte-wise widening fails the round trip.

add_executable(DarwinArgs
    "${CMAKE_CURRENT_SOURCE_DIR}/darwinargs.cpp"
    "${CMAKE_SOURCE_DIR}/code/darwin.cpp"
    "${CMAKE_SOURCE_DIR}/code/utf8.cpp"
    "${CMAKE_SOURCE_DIR}/code/sdl/sdlkeys.cpp"
)
set_target_properties(DarwinArgs PROPERTIES FOLDER "Tests")
opents_test_compile_options(DarwinArgs)
target_compile_options(DarwinArgs PRIVATE -fdeclspec)
target_include_directories(DarwinArgs PRIVATE "${CMAKE_SOURCE_DIR}/code")
target_link_libraries(DarwinArgs PRIVATE SDL3::SDL3-static)
add_test(NAME darwin-args COMMAND DarwinArgs "-DATADIR=/tmp/café")
