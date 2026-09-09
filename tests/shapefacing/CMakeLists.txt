# Shape_Facing_Block is header only, so the harness needs no engine source of its own.

opents_add_test(ShapeFacing
    NAME shapefacing
    SOURCES shapefacing.cpp
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}
    DEFINITIONS WIN32 _WINDOWS _MBCS
    UNPORTED
)
