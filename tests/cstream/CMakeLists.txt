opents_add_test(CStreamContract
    NAME cstream
    SOURCES cstreamcontract.cpp
    ENGINE
        cstream.cpp
        isun_i.c
    DEFINITIONS WIN32 _WINDOWS _MBCS NOMINMAX
    LIBRARIES lzo
    FLOAT
    UNPORTED
)
