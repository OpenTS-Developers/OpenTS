# Tests the UI code that builds without a toolkit: input ownership and text decoding, the
# overlay renderer's arithmetic, image and font-sheet conversion, the reveal animation, and
# the video dirty marks. Linking no UI library keeps the harness cheap and keeps toolkit
# types out of that code.
opents_add_test(UILogic
    NAME uilogic
    SOURCES uilogictest.cpp
    ENGINE
        hsv.cpp
        rgb.cpp
        ui/uiinput.cpp
        ui/uireveal.cpp
        ui/rml/rmlimage.cpp
        ui/rml/rmlfontsheet.cpp
        ui/rml/rmlrendermath.cpp
        videodirty.cpp
)
