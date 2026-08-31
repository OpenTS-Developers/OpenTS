if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    cmake_minimum_required(VERSION 3.23)
    project(NetTimingContract LANGUAGES CXX)
    enable_testing()
    set(OPENTS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../..")
else()
    set(OPENTS_ROOT "${CMAKE_SOURCE_DIR}")
endif()

# Compile the transport timing sources directly so this harness needs no game assets.
add_executable(NetTiming
    "${CMAKE_CURRENT_SOURCE_DIR}/nettiming.cpp"
    "${OPENTS_ROOT}/code/nettime.cpp"
    "${OPENTS_ROOT}/code/nettiming.cpp"
)

target_compile_features(NetTiming PRIVATE cxx_std_20)

target_include_directories(NetTiming PRIVATE "${OPENTS_ROOT}/code")

target_compile_definitions(NetTiming PRIVATE WIN32 _WINDOWS _MBCS)

target_compile_options(NetTiming PRIVATE
    $<$<CONFIG:Debug>:/MTd /EHsc /Zc:__cplusplus>
    $<$<CONFIG:Release>:/MT /EHsc /Zc:__cplusplus>
)

target_link_libraries(NetTiming PRIVATE kernel32 winmm)

set_target_properties(NetTiming PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_test(NAME nettiming COMMAND NetTiming)
