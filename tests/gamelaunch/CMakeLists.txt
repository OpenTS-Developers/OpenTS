# The launch description and the replay messages it reads are compiled straight into the harness.
# None of them depends on the rest of the engine. The entry point stays here so the recursive
# glob building OpenTS cannot pick it up.

opents_add_test(GameLaunchTest
    NAME gamelaunch
    SOURCES gamelaunchtest.cpp
    ENGINE
        launch/gamelaunch.cpp
        proto/wire.cpp
        utf8.cpp
        launch/launchmessages.cpp
        replay/replaymessages.cpp
    DEFINITIONS WIN32 _WINDOWS _MBCS NOMINMAX
    DOCTEST
)
