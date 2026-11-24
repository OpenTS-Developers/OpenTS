
# The pointer swizzler a saved game is loaded through, compiled straight into the harness with
# the logger it reports to. The entry point lives here so the recursive glob building OpenTS
# cannot pick it up.

opents_add_test(SwizzleTest
    NAME swizzle
    SOURCES swizzletest.cpp
    ENGINE
        swizzle.cpp
        dbgprint.cpp
    INCLUDES ${OPENTS_GENERATED_DIR}
    DEFINITIONS WIN32 _WINDOWS _MBCS NOMINMAX
    STAMP
    DOCTEST
)
