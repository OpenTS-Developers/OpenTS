# The key queue is compiled into the harness with stubs for the window, the SDL layer and the frame.

opents_add_test(KeyQueue
    NAME keyqueue
    SOURCES keyqueuecontract.cpp keyqueuestub.cpp
    ENGINE
        keyboard.cpp
    DEFINITIONS WIN32 _WINDOWS NOMINMAX
)
