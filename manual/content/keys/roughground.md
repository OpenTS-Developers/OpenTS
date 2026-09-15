---
key: RoughGround
summary: Ten-tile set of rough ground decorations scattered by the random map generator.
when_omitted:
  kind: value
  value: "-1"
  note: No tile set is selected, so the role stays unresolved.
---

The ten tiles are variants of one decoration and are drawn from evenly. They are unrelated to [`RoughTile`](/keys/roughtile/), which is the plain rough *ground* a whole cell is made of. These are placed as pieces over ground that is already there, and a piece is refused unless every cell it covers is clear ground, or a ramp whose type matches the piece's own. Nothing on that path tests whether a cell is occupied, so a unit or a structure standing there does not turn a placement away.

Nothing outside the random map generator reads the role. The generator scatters between 5 and 30 of them across a map: on desert and mutated maps, and on every tundra and taiga map. It gives up after twenty times as many attempts as pieces wanted, successes counted among them. With the role unresolved, the scatter's picks run from one entry below the start of the tile heap through the theater's first nine tiles. Where the placement test accepts one, the generator lays that tile across the footprint instead of a decoration. A random map is stored as a seed and rebuilt by each player at load time, so every client that plays the map generates that same wrong ground, not just the machine that produced it.
