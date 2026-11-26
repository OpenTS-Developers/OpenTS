# The classes checked here are header-only, so the harness compiles nothing from code/.

opents_add_test(SimDefaults
    NAME simdefaults
    SOURCES simdefaultstest.cpp
    DEFINITIONS WIN32 _WINDOWS _MBCS NOMINMAX
    DOCTEST
)
