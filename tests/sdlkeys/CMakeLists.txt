# Links the static SDL library as the engine does, so a runtime library mismatch fails the link.

opents_add_test(SDLKeysContract
    NAME sdlkeyscontract
    SOURCES sdlkeyscontract.cpp
    ENGINE
        sdl/sdlkeys.cpp
    DEFINITIONS WIN32 _WINDOWS NOMINMAX
    LIBRARIES SDL3::SDL3-static
)
