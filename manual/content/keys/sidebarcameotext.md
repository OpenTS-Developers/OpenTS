---
key: SidebarCameoText
summary: Draws each sidebar cameo's object name across the bottom of the cameo.
see_also: ["system:sidebar"]
when_omitted:
  kind: value
  value: "yes"
---

The caption shows the name alone, wrapped to the width of the slot and broken at spaces and hyphens. The price moves with the setting rather than disappearing: with captions on, the tooltip reports the price by itself; with captions off, it reports the name and the price together. A superweapon's tooltip is its name either way.

The in-game options dialog has the same switch and writes the choice back to `sun.ini`.
