---
key: WallTower
summary: The BuildingType that joins a brick or sandbag wall run from any direction.
see_also: ["system:walls-and-gates", "system:ai-base-building"]
when_omitted:
  kind: value
  value: none
---

Everything the wall tower does follows from being the type this key names; there is no flag for it. While it is standing and undestroyed, the [connection logic](/systems/walls-and-gates/#connection-frames) reads it as a continuation of a brick or sandbag wall from all four directions at once, which is what lets a wall run turn a corner or meet another run through it. Nod wall is a separate family and never connects to it. For a house a human is playing, placing one on a brick or sandbag wall the same house owns removes that wall first, quietly and without refund, while a computer house's tower is placed on top of the surviving segment; and the tower may be placed on an undamaged segment, which no ordinary wall building may do. Once placed it forces its four cardinal neighbors to rebuild their connection frames.

:::caution[Removing a tower damages the wall around it]
When the tower is taken off the map its four cardinal neighbors rebuild their frames, and each of those still holding an undamaged wall is then hit for 200 damage. Any wall whose [`Strength`](/keys/strength/#scope-overlaytype) is 200 or below therefore loses a stage on every side of the tower, which can start the [cascade](/systems/walls-and-gates/#stepping-through-the-stages) along the run.
:::

A node for the acted side's tower precedes every base-defense placeholder a plan carries where that side lists one in [`AIWallTowers`](/keys/aiwalltowers/#scope-side). Only a tower of the type this key names pulls the defense node behind it onto its own cell once placed; any other leaves that node where the plan put it. Owned towers on the acted side's list are kept out of the buildings [defense candidates](/systems/ai-base-building/#base-defenses) test their prerequisites against, and every type on that list is then added back unconditionally, so a defense naming one is buildable before any tower stands.

The value also seeds the first side's [`AIWallTowers`](/keys/aiwalltowers/#scope-side) as each rules file sets it, which is how the computer's GDI bases come to be ringed with towers; an `AIWallTowers=` in that side's own section of the same file overrides it. A base taken over by the computer recognizes wall towers by this type alone, whatever side the house plays for; any other tower is left out of the plan it inherits.
