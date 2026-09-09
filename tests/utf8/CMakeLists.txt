# The helper is compiled straight into the harness. It lives outside code/ so that the
# recursive glob building OpenTS cannot pick this target's entry point up.

opents_add_test(UTF8Contract
    NAME utf8contract
    SOURCES utf8contract.cpp
    ENGINE
        utf8.cpp
    DEFINITIONS WIN32 _WINDOWS NOMINMAX
    UTF8
    UNPORTED
)
