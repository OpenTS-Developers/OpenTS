# The checksum table helpers are defined in statehash.h with no engine dependencies, so the
# harness needs only that header and the checksum engine.
# The entry point stays here so the recursive glob building OpenTS cannot pick it up.

opents_add_test(StateHashTest
    NAME statehash
    SOURCES statehashtest.cpp
    ENGINE
        crc.cpp
    DEFINITIONS WIN32 _WINDOWS _MBCS NOMINMAX
    DOCTEST
)
