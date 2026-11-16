# The replay layer and the engine files it stands on are compiled straight into the harness, so
# none of them may depend on the rest of the engine. The entry point stays in this folder, where
# the recursive glob that builds OpenTS cannot pick it up.

opents_add_test(ReplayTest
    NAME replay
    SOURCES
        hashtest.cpp
        filetest.cpp
        wiretest.cpp
        fieldstest.cpp
        recordstest.cpp
        deflatetest.cpp
        sealtest.cpp
        writertest.cpp
        synthtest.cpp
        readertest.cpp
    ENGINE
        crc.cpp
        sha256.cpp
        protowire.cpp
        deflate.cpp
        rawfile.cpp
        ramfile.cpp
        bfiofile.cpp
        utf8.cpp
        file.cpp
        file_posix.cpp
        file_win.cpp
        replay/replayreader.cpp
        replay/replayrecords.cpp
        replay/replayseal.cpp
        replay/replaywriter.cpp
    INCLUDES
        ${CMAKE_SOURCE_DIR}/thirdparty/bgfx.cmake/bimg/3rdparty/tinyexr/deps
    LIBRARIES bimg
    DEFINITIONS WIN32 _WINDOWS _MBCS NOMINMAX
        REPLAY_PROTO_FILE="${CMAKE_SOURCE_DIR}/docs/replay/replay.proto"
        REPLAY_OUT_DIR="${CMAKE_CURRENT_BINARY_DIR}/out"
    DOCTEST
)
