# The state is fed SDL key events built by hand, so no window is needed; Windows' own key
# state is set with SetKeyboardState.

opents_add_test(SDLInputContract
    NAME sdlinputcontract
    SOURCES sdlinputcontract.cpp
    ENGINE
        sdl/sdlinput.cpp
        sdl/sdlkeys.cpp
    DEFINITIONS WIN32 _WINDOWS NOMINMAX
    LIBRARIES SDL3::SDL3-static
)
