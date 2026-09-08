# The two buffer classes are compiled straight into the harness, along with the surface and
# blit sources their fills reach through. It lives outside code/ so that the recursive glob
# building OpenTS cannot pick this target's entry point up.
add_executable(ZBufRing
    "${CMAKE_CURRENT_SOURCE_DIR}/zbufringtest.cpp"
    "${CMAKE_SOURCE_DIR}/code/zbuffer.cpp"
    "${CMAKE_SOURCE_DIR}/code/abuffer.cpp"
    "${CMAKE_SOURCE_DIR}/code/xsurface.cpp"
    "${CMAKE_SOURCE_DIR}/code/blit.cpp"
    "${CMAKE_SOURCE_DIR}/code/buff.cpp"
    "${CMAKE_SOURCE_DIR}/code/_zbuffer.cpp"
)

target_compile_features(ZBufRing PRIVATE cxx_std_20)

target_include_directories(ZBufRing PRIVATE
    "${CMAKE_SOURCE_DIR}/code"
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

# NOMINMAX matches the engine: blit.cpp reaches for std::min, which the Windows macro eats.
target_compile_definitions(ZBufRing PRIVATE WIN32 _WINDOWS _MBCS NOMINMAX)

target_compile_options(ZBufRing PRIVATE
    $<$<CONFIG:Debug>:/MTd /EHsc /Zc:__cplusplus>
    $<$<CONFIG:Release>:/MT /EHsc /Zc:__cplusplus>
)

target_link_libraries(ZBufRing PRIVATE kernel32 user32 shell32)

set_target_properties(ZBufRing PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_test(NAME zbufring COMMAND ZBufRing)
