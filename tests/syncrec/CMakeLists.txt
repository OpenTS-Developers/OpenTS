# The recorder core is compiled straight into the harness. It lives outside tests/ in code/, but
# has no engine dependencies, so the harness compiles it alone. The entry point stays here so the
# recursive glob building OpenTS cannot pick it up.

opents_add_test(SyncRec
    NAME syncrec
    SOURCES syncrectest.cpp
    ENGINE
        syncrec.cpp
    DEFINITIONS WIN32 _WINDOWS _MBCS NOMINMAX
)
