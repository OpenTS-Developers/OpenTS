# The deployment configuration is compiled straight into the harness, along with the raw
# file class and the INI reader it stands on. It lives outside code/ so that the recursive
# glob building OpenTS cannot pick this target's entry point up.

opents_add_test(DeploymentConfig
    NAME deploymentconfig
    SOURCES deploymentconfigcontract.cpp
    ENGINE
        deploymentconfig.cpp
        _deploymentconfig.cpp
        bfiofile.cpp
        rawfile.cpp
        dbgprint.cpp
        ini.cpp
        utf8.cpp
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
    UNPORTED
)
