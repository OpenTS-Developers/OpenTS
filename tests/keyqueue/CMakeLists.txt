# The key queue is compiled straight into the harness, with stubs in place of the window,
# the platform and the frame, so events are fed to it by hand.

opents_add_test(KeyQueue
    NAME keyqueue
    SOURCES keyqueuecontract.cpp keyqueuestub.cpp
    ENGINE
        keyboard.cpp
    DEFINITIONS WIN32 _WINDOWS NOMINMAX
)
