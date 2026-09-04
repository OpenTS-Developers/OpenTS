add_executable(ActionIds
    "${CMAKE_CURRENT_SOURCE_DIR}/actionidstest.cpp"
)

target_compile_features(ActionIds PRIVATE cxx_std_20)

target_include_directories(ActionIds PRIVATE
    "${CMAKE_SOURCE_DIR}/code"
)

set_target_properties(ActionIds PROPERTIES
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_test(NAME actionids COMMAND ActionIds)
