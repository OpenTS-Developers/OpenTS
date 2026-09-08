add_executable(CStreamContract
    "${CMAKE_CURRENT_SOURCE_DIR}/cstreamcontract.cpp"
    "${CMAKE_SOURCE_DIR}/code/cstream.cpp"
    "${CMAKE_SOURCE_DIR}/code/isun_i.c"
)

target_compile_features(CStreamContract PRIVATE cxx_std_20)

target_include_directories(CStreamContract PRIVATE "${CMAKE_SOURCE_DIR}/code")
target_compile_definitions(CStreamContract PRIVATE WIN32 _WINDOWS _MBCS NOMINMAX)

target_compile_options(CStreamContract PRIVATE
    $<$<CONFIG:Debug>:/MTd /EHsc /Zc:__cplusplus /arch:SSE2 /fp:precise>
    $<$<CONFIG:Release>:/MT /EHsc /Zc:__cplusplus /arch:SSE2 /fp:precise>
)

target_link_libraries(CStreamContract PRIVATE lzo ole32 oleaut32 uuid)

set_target_properties(CStreamContract PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_test(NAME cstream COMMAND CStreamContract)
