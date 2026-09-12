# The colour routines are compiled straight into the harness. It lives outside code/ so that the
# recursive glob building OpenTS cannot pick this target's entry point up.

opents_add_test(ColorOps
    NAME colorops
    SOURCES coloropstest.cpp
    ENGINE
        colorops.cpp
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}
    DEFINITIONS WIN32 _WINDOWS _MBCS
    FLOAT
)
