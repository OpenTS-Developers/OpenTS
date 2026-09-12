# The event pool and everything it reads are compiled straight into the harness. It lives
# outside code/ so that the recursive glob building OpenTS cannot pick this target's entry
# point up.

opents_add_test(AudioEvent
    NAME audioevent
    SOURCES audioevent.cpp
    ENGINE
        audio/audioevent.cpp
        audio/audiomixer.cpp
        audio/audiolevel.cpp
        audio/audiosample.cpp
        audio/audiostream.cpp
        audio/audiodecode.cpp
        audio/audiodevice_null.cpp
        soscodec.cpp
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}
    DEFINITIONS WIN32 _WINDOWS _MBCS
    LIBRARIES miniaudio
    FLOAT
)
