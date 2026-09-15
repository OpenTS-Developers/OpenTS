---
key: Immune
scope: aircrafttype
label: Damage immunity
when_omitted:
  kind: context-dependent
  note: Most object types start at no. An AnimType, BulletType, SmudgeType or VoxelAnimType section starts at yes.
---

Damage processing leaves a runtime instance of an immune type at its current strength and reports no result, so the shot lands and nothing happens. Only damage marked [forced](/keys/c4warhead/) gets through. Paths that must not be resisted still kill an immune object: Tiberium poisons an infantryman, and a demolition charge counts down on a structure.

A TerrainType is reachable only by a warhead marked [`Wood=yes`](/keys/wood/) to begin with, so an immune one has no damage path left at all. A vehicle treats it as impassable rather than repeatedly trying to clear it.

The [low-power damage tick](/systems/power/#the-structure-damage-tick) is not forced, so `Immune=yes` is one way to keep a structure that draws power from decaying while its house is short.
