# The key table is compiled straight into the harness. It links the static SDL library as the
# engine does, so a runtime library mismatch between the two fails the link.

opents_add_test(SDLKeysContract
    NAME sdlkeyscontract
    SOURCES sdlkeyscontract.cpp
    ENGINE
        platform/sdlkeys.cpp
    DEFINITIONS WIN32 _WINDOWS NOMINMAX
    LIBRARIES SDL3::SDL3-static
)
