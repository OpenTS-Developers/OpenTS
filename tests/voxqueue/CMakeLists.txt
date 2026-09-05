# The speech queue is compiled straight into the harness. It lives outside code/ so that the
# recursive glob building OpenTS cannot pick this target's entry point up.
add_executable(VoxQueue
    "${CMAKE_CURRENT_SOURCE_DIR}/voxqueue.cpp"
    "${CMAKE_SOURCE_DIR}/code/voxqueue.cpp"
)

target_compile_features(VoxQueue PRIVATE cxx_std_20)

target_include_directories(VoxQueue PRIVATE
    "${CMAKE_SOURCE_DIR}/code"
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_compile_definitions(VoxQueue PRIVATE WIN32 _WINDOWS _MBCS)

target_compile_options(VoxQueue PRIVATE
    $<$<CONFIG:Debug>:/MTd /EHsc /Zc:__cplusplus>
    $<$<CONFIG:Release>:/MT /EHsc /Zc:__cplusplus>
)

target_link_libraries(VoxQueue PRIVATE kernel32 user32 shell32)

set_target_properties(VoxQueue PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_test(NAME voxqueue COMMAND VoxQueue)
