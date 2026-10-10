# Compile the transport timing sources directly so this harness needs no game assets.
opents_add_test(NetTiming
    NAME nettiming
    SOURCES nettiming.cpp
    ENGINE
        nettime.cpp
        nettiming.cpp
    DEFINITIONS WIN32 _WINDOWS _MBCS
    LIBRARIES winmm
)

opents_add_test(NetConnection
    NAME netconnection
    SOURCES
        connectiontiming.cpp
        connectionstubs.cpp
    ENGINE
        combuf.cpp
        connect.cpp
        netadmit.cpp
        nettime.cpp
        nettiming.cpp
    DEFINITIONS WIN32 _WINDOWS _MBCS NOMINMAX
    LIBRARIES winmm
)
