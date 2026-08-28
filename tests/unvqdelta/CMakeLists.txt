# The decoders are compiled straight into the harness. It lives outside code/ so that the
# recursive glob building OpenTS cannot pick this target's entry point up.
add_executable(UnvqDelta
    "${CMAKE_CURRENT_SOURCE_DIR}/unvqdelta.cpp"
    "${CMAKE_SOURCE_DIR}/code/vqalib/unvq.cpp"
    "${CMAKE_SOURCE_DIR}/code/unvqtblc.cpp"
)

target_compile_features(UnvqDelta PRIVATE cxx_std_20)

target_include_directories(UnvqDelta PRIVATE
    "${CMAKE_SOURCE_DIR}/code"
    "${CMAKE_SOURCE_DIR}/code/vqalib"
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_compile_definitions(UnvqDelta PRIVATE WIN32 _WINDOWS _MBCS)

# The engine builds these sources with SSE2 and precise floating point. The harness matches that
# so a result here carries over to the engine. /arch:SSE2 is the default beyond 32-bit x86 and
# is rejected as an unknown option there, so it is only passed where it applies.
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(UNVQDELTA_ARCH /arch:SSE2)
else()
    set(UNVQDELTA_ARCH "")
endif()

target_compile_options(UnvqDelta PRIVATE
    $<$<CONFIG:Debug>:/MTd /EHsc /Zc:__cplusplus ${UNVQDELTA_ARCH} /fp:precise>
    $<$<CONFIG:Release>:/MT /EHsc /Zc:__cplusplus ${UNVQDELTA_ARCH} /fp:precise>
)

target_link_libraries(UnvqDelta PRIVATE kernel32 user32 shell32)

set_target_properties(UnvqDelta PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_test(NAME unvqdelta COMMAND UnvqDelta)
