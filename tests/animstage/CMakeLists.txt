# AnimStageClass is header-only, so the harness compiles nothing from code/. It supplies the
# game frame counter that the stage timer reads.

opents_add_test(AnimStage
    NAME animstage
    SOURCES animstagetest.cpp
    DEFINITIONS WIN32 _WINDOWS _MBCS NOMINMAX
    DOCTEST
)
