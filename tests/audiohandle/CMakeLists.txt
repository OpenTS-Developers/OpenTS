# The handle encoding is header-only; the harness needs nothing from code/ beyond the
# include path.

opents_add_test(AudioHandle
    NAME audiohandle
    SOURCES audiohandle.cpp
    DEFINITIONS WIN32 _WINDOWS _MBCS
    FLOAT
)
