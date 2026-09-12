# The decoders are compiled straight into the harness. It lives outside code/ so that the
# recursive glob building OpenTS cannot pick this target's entry point up.

opents_add_test(UnvqDelta
    NAME unvqdelta
    SOURCES unvqdelta.cpp
    ENGINE
        vqalib/unvq.cpp
        unvqtblc.cpp
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/code/vqalib
    DEFINITIONS WIN32 _WINDOWS _MBCS
    FLOAT
)
