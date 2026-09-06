# The harness drives the file a saved game is kept in: savefile.cpp's writer and reader over
# the field table, the compressed content, and every refusal the reader makes. It links the
# same vendored LZO library the engine does.

opents_add_test(SaveTest
    NAME save
    SOURCES savetest.cpp
    ENGINE
        savefile.cpp
    DEFINITIONS WIN32 _WINDOWS _MBCS
    LIBRARIES lzo
)
