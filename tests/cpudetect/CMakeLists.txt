# The detection sources are compiled straight into the harness. They live outside tests/ so
# that the recursive glob building OpenTS cannot pick this target's entry point up.
add_executable(CpuDetect
    "${CMAKE_CURRENT_SOURCE_DIR}/cpudetect.cpp"
    "${CMAKE_SOURCE_DIR}/code/getcpu.cpp"
    "${CMAKE_SOURCE_DIR}/code/mpu.cpp"
)

target_compile_features(CpuDetect PRIVATE cxx_std_20)

target_include_directories(CpuDetect PRIVATE
    "${CMAKE_SOURCE_DIR}/code"
)

target_compile_definitions(CpuDetect PRIVATE WIN32 _WINDOWS _MBCS NOMINMAX)

# The engine builds these sources with SSE2 and precise floating point. The harness matches
# that so a result here carries over to the engine.
target_compile_options(CpuDetect PRIVATE
    $<$<CONFIG:Debug>:/MTd /EHsc /Zc:__cplusplus /arch:SSE2 /fp:precise>
    $<$<CONFIG:Release>:/MT /EHsc /Zc:__cplusplus /arch:SSE2 /fp:precise>
)

target_link_libraries(CpuDetect PRIVATE kernel32 user32 shell32)

set_target_properties(CpuDetect PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_test(NAME cpudetect COMMAND CpuDetect)
