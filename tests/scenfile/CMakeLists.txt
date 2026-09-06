# The scenario file holder is compiled straight into the harness. It lives in code/ but has
# no engine dependencies, so the harness compiles it alone. The entry point stays here so
# the recursive glob building OpenTS cannot pick it up.
add_executable(ScenFile
    "${CMAKE_CURRENT_SOURCE_DIR}/scenfilecontract.cpp"
    "${CMAKE_SOURCE_DIR}/code/scenfile.cpp"
)

target_compile_features(ScenFile PRIVATE cxx_std_20)

target_include_directories(ScenFile PRIVATE
    "${CMAKE_SOURCE_DIR}/code"
)

target_compile_definitions(ScenFile PRIVATE WIN32 _WINDOWS _MBCS NOMINMAX)

target_compile_options(ScenFile PRIVATE
    $<$<CONFIG:Debug>:/MTd /EHsc /Zc:__cplusplus>
    $<$<CONFIG:Release>:/MT /EHsc /Zc:__cplusplus>
)

target_link_libraries(ScenFile PRIVATE kernel32 user32 shell32)

set_target_properties(ScenFile PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_test(NAME scenfile COMMAND ScenFile)
