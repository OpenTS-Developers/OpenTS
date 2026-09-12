# The LZO block classes are compiled straight into the harness against the same vendored
# library the engine links. It lives outside code/ so that the recursive glob building OpenTS
# cannot pick this target's entry point up.

opents_add_test(LzoBlock
    NAME lzoblock
    SOURCES lzoblock.cpp
    ENGINE
        buff.cpp
        lzopipe.cpp
        lzostraw.cpp
        pipe.cpp
        straw.cpp
        xpipe.cpp
        xstraw.cpp
    DEFINITIONS WIN32 _WINDOWS _MBCS NOMINMAX
    LIBRARIES lzo
    FLOAT
)
