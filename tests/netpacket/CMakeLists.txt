# The packet decoder is compiled directly into this asset-free contract harness so malformed
# wire data can be exercised without starting the engine or opening a network session.
add_executable(NetContract
    "${CMAKE_CURRENT_SOURCE_DIR}/netcontract.cpp"
    "${CMAKE_SOURCE_DIR}/code/_event.cpp"
    "${CMAKE_SOURCE_DIR}/code/netadmit.cpp"
    "${CMAKE_SOURCE_DIR}/code/netglobal.cpp"
    "${CMAKE_SOURCE_DIR}/code/netpacket.cpp"
    "${CMAKE_SOURCE_DIR}/code/netreader.cpp"
    "${CMAKE_SOURCE_DIR}/code/netsemantic.cpp"
)

target_compile_features(NetContract PRIVATE cxx_std_20)

target_include_directories(NetContract PRIVATE
    "${CMAKE_SOURCE_DIR}/code"
    "${OPENTS_GENERATED_DIR}"
)

add_dependencies(NetContract OpenTSBuildStamp)

target_compile_definitions(NetContract PRIVATE WIN32 _WINDOWS _MBCS NOMINMAX)

target_compile_options(NetContract PRIVATE
    $<$<CONFIG:Debug>:/MTd /EHsc /Zc:__cplusplus>
    $<$<CONFIG:Release>:/MT /EHsc /Zc:__cplusplus>
)

set_target_properties(NetContract PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_test(NAME netpacket COMMAND NetContract)
