# The toolkit-free UI state: the input ownership the shell routes messages by, the text
# decoding it feeds the documents, the checks the overlay renderer makes before it draws,
# the art it turns into pixels, and the marks the presenter owes the screen, compiled
# without any UI library so the harness stays cheap and the code stays free of toolkit
# types.
opents_add_test(UILogic
    NAME uilogic
    SOURCES uilogictest.cpp
    ENGINE
        hsv.cpp
        rgb.cpp
        ui/uiinput.cpp
        ui/rml/rmlimage.cpp
        ui/rml/rmlfontsheet.cpp
        ui/rml/rmlrendermath.cpp
        videodirty.cpp
)
