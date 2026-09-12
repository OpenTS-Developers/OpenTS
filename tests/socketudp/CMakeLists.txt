# The UDP transport is compiled directly into this asset-free harness and driven through a
# socket that reaches no network, so the send and receive passes can be exercised without
# opening a session.

opents_add_test(SocketUdp
    NAME socketudp
    SOURCES socketudp.cpp
    ENGINE
        ipxaddr.cpp
        netadmit.cpp
        netsocket_null.cpp
        wsproto.cpp
        wspudp.cpp
    INCLUDES ${OPENTS_GENERATED_DIR}
    DEFINITIONS WIN32 _WINDOWS _MBCS NOMINMAX
    LIBRARIES ws2_32
    STAMP
)
