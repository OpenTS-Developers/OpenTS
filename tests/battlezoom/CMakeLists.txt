# The geometry, the pending zoom state and the interface scale are tested without game assets.

opents_add_test(BattleZoom
    NAME battlezoom
    SOURCES battlezoomcontract.cpp
    ENGINE
        battlezoomlayout.cpp
        battlezoomstate.cpp
        uiscalelayout.cpp
    DEFINITIONS WIN32 _WINDOWS NOMINMAX
)
