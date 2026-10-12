# The battlefield zoom geometry is compiled into the harness on its own; it reads no engine state.

opents_add_test(BattleZoom
    NAME battlezoom
    SOURCES battlezoomcontract.cpp
    ENGINE
        battlezoomlayout.cpp
    DEFINITIONS WIN32 _WINDOWS NOMINMAX
)
