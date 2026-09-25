# Runs the real SDL layer over a small window with a recording stand-in for the game's
# handler. The test needs a desktop session for the window.
opents_add_test(SDLWindow
    NAME sdlwindow
    SOURCES sdlwindowcontract.cpp sdlwindowstub.cpp
    ENGINE
        sdl/sdlwindow.cpp
        sdl/sdlevents.cpp
        sdl/sdlkeys.cpp
        utf8.cpp
    DEFINITIONS WIN32 _WINDOWS NOMINMAX
    LIBRARIES SDL3::SDL3-static comctl32
    UTF8
)
