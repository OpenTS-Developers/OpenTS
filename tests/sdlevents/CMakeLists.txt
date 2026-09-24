# The SDL event translator and the key table are compiled straight into the harness. The
# events are built by hand, so SDL is linked but never started.

opents_add_test(SDLEventsContract
    NAME sdleventscontract
    SOURCES sdleventscontract.cpp
    ENGINE
        platform/sdlevents.cpp
        platform/sdlkeys.cpp
        utf8.cpp
    DEFINITIONS WIN32 _WINDOWS NOMINMAX
    LIBRARIES SDL3::SDL3-static
    UTF8
)
