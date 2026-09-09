# The scenario file holder is compiled straight into the harness. It lives in code/ but has
# no engine dependencies, so the harness compiles it alone. The entry point stays here so
# the recursive glob building OpenTS cannot pick it up.

opents_add_test(ScenFile
    NAME scenfile
    SOURCES scenfilecontract.cpp
    ENGINE
        scenfile.cpp
    DEFINITIONS WIN32 _WINDOWS _MBCS NOMINMAX
    UNPORTED
)
