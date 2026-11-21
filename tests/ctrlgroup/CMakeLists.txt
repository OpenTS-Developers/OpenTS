# The control group table is header-only, so the harness compiles nothing from code/. It
# supplies a stand-in techno, which the table only compares.

opents_add_test(ControlGroupTest
    NAME ctrlgroup
    SOURCES ctrlgrouptest.cpp
    DEFINITIONS WIN32 _WINDOWS _MBCS NOMINMAX
    DOCTEST
)
