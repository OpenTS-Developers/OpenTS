# The harness links the vendored UI toolkits the way the engine does and loads the shipped
# documents through a recording render interface, so a runtime library mismatch, a document
# that fails to parse or a style outside the implemented render methods fails here. The
# documents are read from the source tree, so it needs no run directory; the string-name
# table it resolves them with is generated per build by the stamp target. The shell itself
# is built in over a host the harness controls and a stub in place of the developer overlays.
opents_add_test(UIShell
    NAME uishell
    SOURCES uishell.cpp uidevstub.cpp
    ENGINE
        ui/uiscreen.cpp
        ui/uiinput.cpp
        ui/uishell.cpp
        ui/uiunicode.cpp
        ui/rml/rmlkeys.cpp
        ui/rml/rmlrendermath.cpp
        ui/rml/rmlsystem.cpp
        ui/rml/rmlview.cpp
        ui/screens/display/uidisplay.cpp
        ui/screens/gamectrl/uigamectrl.cpp
        ui/screens/keyboard/uikeyboard.cpp
        ui/screens/mainopt/uimainopt.cpp
        ui/screens/msgbox/uimsgbox.cpp
        ui/screens/sound/uisound.cpp
        ui/screens/version/uiversion.cpp
        ui/screens/waitbox/uiwaitbox.cpp
    INCLUDES "${OPENTS_GENERATED_DIR}"
    DEFINITIONS WIN32 _WINDOWS NOMINMAX "OPENTS_UI_DIR=\"${CMAKE_SOURCE_DIR}/ui\""
    LIBRARIES RmlUi::Core freetype imgui
    STAMP
)
