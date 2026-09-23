# HouseSet and HouseArray are header-only, so the harness compiles nothing from code/. It
# supplies its own stand-in house, which carries nothing but the heap index they read.

opents_add_test(HouseSet
    NAME houseset
    SOURCES housesettest.cpp
    DEFINITIONS WIN32 _WINDOWS _MBCS NOMINMAX
)
