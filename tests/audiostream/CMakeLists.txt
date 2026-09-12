# The streams, the feeder and everything they use are compiled straight into the harness.
# It lives outside code/ so that the recursive glob building OpenTS cannot pick this
# target's entry point up.

opents_add_test(AudioStream
    NAME audiostream
    SOURCES audiostream.cpp
    ENGINE
        audio/audiostream.cpp
        audio/audiomixer.cpp
        audio/audiolevel.cpp
        audio/audiosample.cpp
        audio/audiodecode.cpp
        audio/audiodevice_null.cpp
        soscodec.cpp
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}
    DEFINITIONS WIN32 _WINDOWS _MBCS
    LIBRARIES miniaudio
    FLOAT
)
