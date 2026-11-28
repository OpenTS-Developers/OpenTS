# The harness supplies its own file class and uses the same miniz target as the engine.

opents_add_test(PngWrite
    NAME png
    SOURCES pngcontract.cpp
    ENGINE
        writepng.cpp
    LIBRARIES opents_miniz
    DEFINITIONS WIN32 _WINDOWS _MBCS NOMINMAX
    DOCTEST
)
