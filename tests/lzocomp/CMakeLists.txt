opents_add_test(LzoComp
    NAME lzocomp
    SOURCES lzocomp.cpp
    ENGINE
        lzopipe.cpp
        lzostraw.cpp
        pipe.cpp
        straw.cpp
    DEFINITIONS WIN32 _WINDOWS _MBCS NOMINMAX
    LIBRARIES lzo
)
