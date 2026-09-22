# The harness supplies its own file class, and miniz comes from bimg as in the engine.

opents_add_test(PngWrite
    NAME png
    SOURCES pngcontract.cpp
    ENGINE
        writepng.cpp
    INCLUDES
        ${CMAKE_SOURCE_DIR}/thirdparty/bgfx.cmake/bimg/3rdparty/tinyexr/deps
    LIBRARIES bimg
    DEFINITIONS WIN32 _WINDOWS _MBCS NOMINMAX
)
