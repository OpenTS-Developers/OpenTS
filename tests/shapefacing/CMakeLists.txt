# Shape_Facing_Block is header only, so the harness needs no engine source of its own.
add_executable(ShapeFacing
    "${CMAKE_CURRENT_SOURCE_DIR}/shapefacing.cpp"
)

target_compile_features(ShapeFacing PRIVATE cxx_std_20)

target_include_directories(ShapeFacing PRIVATE
    "${CMAKE_SOURCE_DIR}/code"
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_compile_definitions(ShapeFacing PRIVATE WIN32 _WINDOWS _MBCS)

target_compile_options(ShapeFacing PRIVATE
    $<$<CONFIG:Debug>:/MTd /EHsc /Zc:__cplusplus>
    $<$<CONFIG:Release>:/MT /EHsc /Zc:__cplusplus>
)

target_link_libraries(ShapeFacing PRIVATE kernel32 user32 shell32)

set_target_properties(ShapeFacing PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_test(NAME shapefacing COMMAND ShapeFacing)
