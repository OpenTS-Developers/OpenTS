# The main window's message translator is compiled straight into the harness with the UTF-8
# decoder it shares with the interface, so no window or message loop is needed.

opents_add_test(WinEventContract
    NAME wineventcontract
    SOURCES wineventcontract.cpp
    ENGINE
        winevent.cpp
        ui/uiinput.cpp
    DEFINITIONS WIN32 _WINDOWS NOMINMAX
)
