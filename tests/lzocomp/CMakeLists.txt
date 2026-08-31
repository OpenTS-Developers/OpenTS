# The compression sources are compiled straight into the harness. They live outside code/
# so that the recursive glob building OpenTS cannot pick this target's entry point up.
add_executable(LzoComp
    "${CMAKE_CURRENT_SOURCE_DIR}/lzocomp.cpp"
    "${CMAKE_SOURCE_DIR}/code/lzo1x_c.cpp"
    "${CMAKE_SOURCE_DIR}/code/lzo1x_d.cpp"
    "${CMAKE_SOURCE_DIR}/code/lzopipe.cpp"
    "${CMAKE_SOURCE_DIR}/code/lzostraw.cpp"
    "${CMAKE_SOURCE_DIR}/code/pipe.cpp"
    "${CMAKE_SOURCE_DIR}/code/straw.cpp"
)

target_compile_features(LzoComp PRIVATE cxx_std_20)

target_include_directories(LzoComp PRIVATE
    "${CMAKE_SOURCE_DIR}/code"
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_compile_definitions(LzoComp PRIVATE WIN32 _WINDOWS _MBCS NOMINMAX)

target_compile_options(LzoComp PRIVATE
    $<$<CONFIG:Debug>:/MTd /EHsc /Zc:__cplusplus>
    $<$<CONFIG:Release>:/MT /EHsc /Zc:__cplusplus>
)

target_link_libraries(LzoComp PRIVATE kernel32 user32 shell32)

set_target_properties(LzoComp PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_test(NAME lzocomp COMMAND LzoComp)
