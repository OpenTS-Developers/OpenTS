
# The harness reads the source tree when it runs, so an edit to code/ is checked without a rebuild.
# A failing run writes the current hashes to functions.txt in this build folder.

opents_add_test(SaveSerialTest
    NAME saveserial
    SOURCES saveserialtest.cpp
    DEFINITIONS
        OPENTS_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
        SAVESERIAL_RECORD="${CMAKE_CURRENT_SOURCE_DIR}/functions.txt"
        SAVESERIAL_CURRENT="${CMAKE_CURRENT_BINARY_DIR}/functions.txt"
    DOCTEST
)
