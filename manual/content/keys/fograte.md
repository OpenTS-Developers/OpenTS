---
key: FogRate
summary: Game minutes between fog of war regrowth passes.
see_also: ["system:map-visibility", ShroudRate]
when_omitted:
  kind: value
  value: ".05"
---

```ini title="rules.ini"
[AudioVisual]
FogRate=.1 ; a regrowth pass every 90 frames
```

The value is converted to frames at 900 frames to the game minute, so the default gives a pass every 45 frames. `FogRate=0` stops the fog from ever closing back in, leaving whatever has been uncovered permanently clear.

The timer starts a scenario at zero, so the first eligible frame runs a pass immediately. Nothing happens on any of these passes unless the game options switched fog of war on; [fog regrowth](/systems/map-visibility/#fog-regrowth) covers what one pass does.
