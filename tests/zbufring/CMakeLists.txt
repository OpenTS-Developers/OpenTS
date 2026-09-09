# The two buffer classes are compiled straight into the harness, along with the surface and
# blit sources their fills reach through. It lives outside code/ so that the recursive glob
# building OpenTS cannot pick this target's entry point up.

opents_add_test(ZBufRing
    NAME zbufring
    SOURCES zbufringtest.cpp
    ENGINE
        zbuffer.cpp
        abuffer.cpp
        xsurface.cpp
        blit.cpp
        buff.cpp
        _zbuffer.cpp
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}
    DEFINITIONS WIN32 _WINDOWS _MBCS NOMINMAX
    UNPORTED
)
