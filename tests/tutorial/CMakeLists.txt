# The tutorial text table is compiled straight into the harness along with the INI reader it
# takes its lines from. The entry point lives here so the recursive glob building OpenTS cannot
# pick it up.

opents_add_test(TutorialText
    NAME tutorialtext
    SOURCES tutorialcontract.cpp
    ENGINE
        tutorial.cpp
        ini.cpp
        utf8.cpp
        dbgprint.cpp
        b64pipe.cpp
        b64straw.cpp
        base64.cpp
        buff.cpp
        crc.cpp
        cstraw.cpp
        int.cpp
        mpmath.cpp
        pipe.cpp
        pk.cpp
        straw.cpp
        trim.cpp
        xpipe.cpp
        xstraw.cpp
    INCLUDES ${OPENTS_GENERATED_DIR}
    DEFINITIONS WIN32 _WINDOWS _MBCS NOMINMAX
    UTF8
    STAMP
)
