# Mounts toy MIX archives and checks which copy a shared name resolves to. The harness
# builds the mix and file classes itself, and every archive it opens is one it writes.

opents_add_test(MixOrder
    NAME mixorder
    SOURCES mixorder.cpp
    ENGINE
        mixfile.cpp
        ccfile.cpp
        cdfile.cpp
        bfiofile.cpp
        rawfile.cpp
        file.cpp
        file_posix.cpp
        file_win.cpp
        dbgprint.cpp
        utf8.cpp
        buff.cpp
        crc.cpp
        cstraw.cpp
        straw.cpp
        xstraw.cpp
        pipe.cpp
        xpipe.cpp
        pk.cpp
        pkstraw.cpp
        blwstraw.cpp
        blowfish.cpp
        int.cpp
        mpmath.cpp
        rndstraw.cpp
        sha.cpp
        shastraw.cpp
        trim.cpp
    INCLUDES ${OPENTS_GENERATED_DIR}
    DEFINITIONS WIN32 _WINDOWS _MBCS NOMINMAX
    UTF8
    STAMP
    DOCTEST
)
