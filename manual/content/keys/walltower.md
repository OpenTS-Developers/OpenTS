---
key: WallTower
summary: The BuildingType that joins a brick or sandbag wall run from any direction.
see_also: ["system:walls-and-gates", "system:ai-base-building"]
when_omitted:
  kind: value
  value: none
---

Everything the wall tower does follows from being the type this key names; there is no flag for it. While it is standing and undestroyed, the [connection logic](/systems/walls-and-gates/#connection-frames) reads it as a continuation of a brick or sandbag wall from all four directions at once. That is what lets a wall run turn a corner or meet another run through it. Nod wall is a separate family and never connects to it. For a house a human is playing, placing one on a brick or sandbag wall the same house owns removes that wall first, quietly and without refund. A computer house's tower is placed on top of the surviving segment. The tower may also be placed on an undamaged segment, which no ordinary wall building may do. Once placed it forces its four cardinal neighbors to rebuild their connection frames.

:::caution[Removing a tower damages the wall around it]
When the tower is taken off the map its four cardinal neighbors rebuild their frames, and each of those still holding an undamaged wall is then hit for 200 damage. Any wall whose [`Strength`](/keys/strength/#scope-overlaytype) is 200 or below therefore loses a stage on every side of the tower, which can start the [cascade](/systems/walls-and-gates/#stepping-through-the-stages) along the run.
:::

The computer builds its base from a plan, an ordered list of entries, some of which are base-defense slots to be filled later. Where the side this house acts as lists a tower in [`AIWallTowers`](/keys/aiwalltowers/), a node for that tower goes into the plan just ahead of every base-defense slot. When a tower is placed, the plan's next base-defense slot is moved onto the tower's cell, but only when the tower is the type this key names; any other tower leaves that slot where the plan put it. A defense naming one of these towers is buildable before any tower stands. Owned towers on the acted side's list are kept out of the buildings the [defense candidates](/systems/ai-base-building/#base-defenses) test their prerequisites against, and every type on that list is then added back unconditionally.

The value also seeds the first side's [`AIWallTowers`](/keys/aiwalltowers/) as each rules file sets it, which is how the computer's GDI bases come to be ringed with towers. An `AIWallTowers=` in that side's own section of the same file overrides it. A base taken over by the computer recognizes wall towers by this type alone, whatever side the house plays for; any other tower is left out of the plan it inherits.
