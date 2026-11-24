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
        vectorstest.cpp
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
        REPLAY_VECTORS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/vectors"
    DOCTEST
)

# With Python 3.11 or later, CTest also runs the Python reference reader's tests and has
# it verify each file the synthetic case writes. Without it these cases are not registered.
find_package(Python3 3.11 COMPONENTS Interpreter)
if(Python3_Interpreter_FOUND)
    add_test(NAME "replay.python reader tests"
        COMMAND "${Python3_EXECUTABLE}" -m unittest discover -s tools/replay/tests
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    )

    foreach(name IN ITEMS minimal three-intervals interval-boundary embedded-map checkpoint-frame0
            sealed-payload unclosed)
        # Exit code 3 is a file without END whose seals all match.
        set(expect 0)
        if(name STREQUAL "unclosed")
            set(expect 3)
        endif()
        add_test(NAME "replay.python verifies ${name}.cncr"
            COMMAND "${CMAKE_COMMAND}"
                -D "PYTHON=${Python3_EXECUTABLE}"
                -D "SCRIPT=${CMAKE_SOURCE_DIR}/tools/replay/cncreplay.py"
                -D "FILE=${CMAKE_CURRENT_BINARY_DIR}/out/${name}.cncr"
                -D "EXPECT=${expect}"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/verifyexit.cmake"
        )
        set_tests_properties("replay.python verifies ${name}.cncr" PROPERTIES FIXTURES_REQUIRED replay_synthetic)
    endforeach()

    # The synthetic case is registered only once the harness is built, so its fixture is set in a
    # file that CTest reads after the case list.
    set(fixture_file "${CMAKE_CURRENT_BINARY_DIR}/ReplayTest_fixtures.cmake")
    file(WRITE "${fixture_file}"
        "if(EXISTS \"${CMAKE_CURRENT_BINARY_DIR}/ReplayTest_tests-\${CTEST_CONFIGURATION_TYPE}.cmake\")\n"
        "    set_tests_properties(\"replay.synthetic replay files are written for other readers\"\n"
        "        PROPERTIES FIXTURES_SETUP replay_synthetic)\n"
        "endif()\n"
    )
    set_property(DIRECTORY APPEND PROPERTY TEST_INCLUDE_FILES "${fixture_file}")
else()
    message(STATUS "Python 3.11 or later not found: the replay cross-check cases are not registered")
endif()
