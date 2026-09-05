# The out-of-sync bookkeeping and the pending multiplayer load are compiled straight into the
# harness. They live in code/ but have no engine dependencies, so the harness compiles them
# alone. The entry point stays here so the recursive glob building OpenTS cannot pick it up.
add_executable(Desync
    "${CMAKE_CURRENT_SOURCE_DIR}/desynccontract.cpp"
    "${CMAKE_SOURCE_DIR}/code/desync.cpp"
    "${CMAKE_SOURCE_DIR}/code/mpload.cpp"
)

target_compile_features(Desync PRIVATE cxx_std_20)

target_include_directories(Desync PRIVATE
    "${CMAKE_SOURCE_DIR}/code"
)

target_compile_definitions(Desync PRIVATE WIN32 _WINDOWS _MBCS NOMINMAX)

target_compile_options(Desync PRIVATE
    $<$<CONFIG:Debug>:/MTd /EHsc /Zc:__cplusplus>
    $<$<CONFIG:Release>:/MT /EHsc /Zc:__cplusplus>
)

target_link_libraries(Desync PRIVATE kernel32 user32 shell32)

set_target_properties(Desync PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_test(NAME desync COMMAND Desync)
