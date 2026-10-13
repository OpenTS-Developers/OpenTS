# The geometry and pending zoom state are tested without game assets.

opents_add_test(BattleZoom
    NAME battlezoom
    SOURCES battlezoomcontract.cpp
    ENGINE
        battlezoomlayout.cpp
        battlezoomstate.cpp
    DEFINITIONS WIN32 _WINDOWS NOMINMAX
)
