# The LCW block classes and their buffer terminators are compiled straight into the harness. It
# lives outside code/ so that the recursive glob building OpenTS cannot pick this target's entry
# point up.

opents_add_test(LcwBlock
    NAME lcwblock
    SOURCES lcwblock.cpp
    ENGINE
        buff.cpp
        lcw.cpp
        lcwpipe.cpp
        lcwstraw.cpp
        pipe.cpp
        straw.cpp
        xpipe.cpp
        xstraw.cpp
    DEFINITIONS WIN32 _WINDOWS _MBCS NOMINMAX
    FLOAT
)
