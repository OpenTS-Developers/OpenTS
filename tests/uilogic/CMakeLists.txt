# The toolkit-free UI state: the input ownership the shell routes messages by, the text
# decoding it feeds the documents, and the marks the presenter owes the screen, compiled
# without any UI library so the harness stays cheap and the code stays free of toolkit types.
opents_add_test(UILogic
    NAME uilogic
    SOURCES uilogictest.cpp
    ENGINE
        ui/uiinput.cpp
        videodirty.cpp
)
