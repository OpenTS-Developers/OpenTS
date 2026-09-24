# Runs the real SDL platform over a small main window and hands its events to a recording
# sink, so the pump's ordering and state are checked as the game sees them. The test needs a
# desktop session for the window.
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
