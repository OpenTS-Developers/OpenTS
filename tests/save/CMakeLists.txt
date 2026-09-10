# The harness drives the file a saved game is kept in: savefile.cpp's writer and reader over
# the field table, the compressed content, and every refusal the reader makes. It links the
# same vendored LZO library the engine does, and compiles crc.cpp for the checksum the
# header and the content carry.

opents_add_test(SaveTest
    NAME save
    SOURCES savetest.cpp
    ENGINE
        crc.cpp
        savefile.cpp
    DEFINITIONS WIN32 _WINDOWS _MBCS NOMINMAX
    LIBRARIES lzo
)
