# Zoom geometry, state and tooltip polling are tested without game assets or a window.

opents_add_test(BattleZoom
    NAME battlezoom
    SOURCES battlezoomcontract.cpp battlezoomtooltip.cpp
    ENGINE
        battlezoomlayout.cpp
        battlezoomstate.cpp
        uiscalelayout.cpp
        tooltip.cpp
    DEFINITIONS WIN32 _WINDOWS NOMINMAX
    LIBRARIES winmm
)
