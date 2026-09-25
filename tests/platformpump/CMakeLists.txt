# Runs the real SDL platform over a small window with a recording stand-in for the game's
# handler. The test needs a desktop session for the window.
opents_add_test(PlatformPump
    NAME platformpump
    SOURCES platformpumpcontract.cpp platformpumpstub.cpp
    ENGINE
        platform/sdlplatform.cpp
        platform/sdlevents.cpp
        platform/sdlkeys.cpp
        utf8.cpp
    DEFINITIONS WIN32 _WINDOWS NOMINMAX
    LIBRARIES SDL3::SDL3-static comctl32
    UTF8
)
