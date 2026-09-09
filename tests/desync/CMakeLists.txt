# The out-of-sync bookkeeping and the pending multiplayer load are compiled straight into the
# harness. They live in code/ but have no engine dependencies, so the harness compiles them
# alone. The entry point stays here so the recursive glob building OpenTS cannot pick it up.

opents_add_test(Desync
    NAME desync
    SOURCES desynccontract.cpp
    ENGINE
        desync.cpp
        mpload.cpp
    DEFINITIONS WIN32 _WINDOWS _MBCS NOMINMAX
)
