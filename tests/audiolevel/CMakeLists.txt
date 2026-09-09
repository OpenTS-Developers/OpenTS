# The level code is compiled straight into the harness. It lives outside code/ so that the
# recursive glob building OpenTS cannot pick this target's entry point up.

opents_add_test(AudioLevel
    NAME audiolevel
    SOURCES audiolevel.cpp
    ENGINE
        audio/audiolevel.cpp
    DEFINITIONS WIN32 _WINDOWS _MBCS
    FLOAT
)
