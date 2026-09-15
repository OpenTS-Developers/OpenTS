---
key: Ice1Set
summary: First of the three sixty-four-tile ice sets a snow theater provides.
see_also: [Ice2Set, Ice3Set, IceShoreSet]
when_omitted:
  kind: value
  value: "-1"
  note: No tile set is selected, so the role stays unresolved.
---

The value is a tile set number, counted from zero: a value of `3` selects the set declared in section `[TileSet0003]` of the theater control file. The sections are read in number order from `[TileSet0000]`, and the read stops at the first number with no section.

Ice comes in three sets of the same shape, and every sixty-four-tile set is laid out identically. Offsets 0 through 15 are full ice, offset 16 is the cracked tile, and offsets 17 through 63 are the edge pieces that run ice up against everything else. The engine reads those positions as fixed offsets from the set's first tile, so replacement artwork must keep the same layout. The three exist only so that a frozen stretch does not repeat one pattern. Whenever the engine lays ice it picks one of the three at random, so a theater that resolves some but not all of them will place unresolved ice a third or two thirds of the time.

Which variant a full-ice cell takes is decided from its four orthogonal neighbors much as blending works elsewhere, except that the neighbors being tested for are full ice rather than a ground type. The write is shifted: the pass lays offset 0 when all four neighbors are full ice. Otherwise it lays one more than the pattern value, which runs from 1 to 15, so the blended variants occupy offsets 2 through 16 and offset 1 is never written. A cell with no ice around it has pattern 15 and lands on offset 16, the cracked tile. Edge pieces are chosen from a pattern of all eight neighbors instead: a neighbor counts when it is not open water, an edge piece, or a shoreline piece. The lookup is shared with [`IceShoreSet`](/keys/iceshoreset/) on the land side of the same boundary.

All of this runs only where the theater has ice growth enabled (`IsIceGrowthEnabled`), and none of it is gated on the role resolving.

Ice thickens and refreezes only while the scenario has ice growth turned on, but cracking does not: a vehicle crossing ice or an explosion over it can crack and break a sheet in any snow scenario.
