# The session options a scenario applies to the special flags are header-only, so the harness
# compiles nothing from code/.

opents_add_test(SpecialFlags
    NAME specialflags
    SOURCES specialflagscontract.cpp
    DEFINITIONS WIN32 _WINDOWS _MBCS NOMINMAX
)
