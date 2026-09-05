# The helper is compiled straight into the harness. It lives outside code/ so that the
# recursive glob building OpenTS cannot pick this target's entry point up.
add_executable(UTF8Contract
    "${CMAKE_CURRENT_SOURCE_DIR}/utf8contract.cpp"
    "${CMAKE_SOURCE_DIR}/code/utf8.cpp"
)
target_compile_features(UTF8Contract PRIVATE cxx_std_20)
target_include_directories(UTF8Contract PRIVATE "${CMAKE_SOURCE_DIR}/code")
target_compile_definitions(UTF8Contract PRIVATE WIN32 _WINDOWS NOMINMAX)
# The engine compiles its sources as UTF-8. The harness matches that so a literal here
# means the same bytes it would in the engine.
target_compile_options(UTF8Contract PRIVATE
    /utf-8
    $<$<CONFIG:Debug>:/MTd /EHsc /Zc:__cplusplus>
    $<$<CONFIG:Release>:/MT /EHsc /Zc:__cplusplus>
)
target_link_libraries(UTF8Contract PRIVATE kernel32)
set_target_properties(UTF8Contract PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
add_test(NAME utf8contract COMMAND UTF8Contract)
