# The packet decoder is compiled directly into this asset-free contract harness so malformed
# wire data can be exercised without starting the engine or opening a network session.

opents_add_test(NetContract
    NAME netpacket
    SOURCES netcontract.cpp
    ENGINE
        _event.cpp
        mpload.cpp
        netadmit.cpp
        netglobal.cpp
        netpacket.cpp
        netreader.cpp
        netsemantic.cpp
    INCLUDES ${OPENTS_GENERATED_DIR}
    DEFINITIONS WIN32 _WINDOWS _MBCS NOMINMAX
    STAMP
    UNPORTED
)
