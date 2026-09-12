# The decoders are compiled straight into the harness. It lives outside code/ so that the
# recursive glob building OpenTS cannot pick this target's entry point up.

opents_add_test(AudDecode
    NAME auddecode
    SOURCES auddecode.cpp
    ENGINE
        audio/audiodecode.cpp
        soscodec.cpp
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}
    DEFINITIONS WIN32 _WINDOWS _MBCS
    LIBRARIES miniaudio
    FLOAT
)
