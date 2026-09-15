---
key: CrystalCliff
summary: Tile set holding the crystal cliff pieces.
see_also: [CliffSet, CrystalTile, ClearToCrystalLat]
when_omitted:
  kind: value
  value: "-1"
  note: The role stays unresolved, because no tile set number can match it, and the transition pass then reads the theater's first, fourth and fifth tiles as crystal-cliff halves.
---

Only half of a crystal cliff tile is crystal, and the pass that dresses crystal ground against clear ground knows which half. A subtile is one cell of a multi-cell tile piece, numbered across each row from the top left. A neighboring cell counts as crystal when it has any of:

- the first piece on subtile `0` or `1`,
- the second piece on subtile `2` or above,
- the fifth piece on an odd subtile,
- the sixth piece on an even subtile.

On any other subtile of those four pieces, and on every other piece of the set, the neighbor counts as clear and the crystal cell beside it takes a transition edge from [`ClearToCrystalLat`](/keys/cleartocrystallat/) instead of staying solid. [Theater control files](/formats/theater-control/) explains how a `[General]` role is resolved to a live tile index.

Random map generation reads the role a second way. With Firestorm enabled on a mutated-biome map, the crystal deposit pass hangs a formation off a suitable cliff by placing the piece at this role plus `0`, `1`, `4` or `5`. In two places the cliff pass would pick an inside corner facing north-west or a lone north-west outside corner: six of its forty shapes between them. There it substitutes a crystal face for the ordinary one at a one-in-twenty chance.

:::danger[The cliff-pass substitution assumes the two sets are neighbors]
That substitution does not place the crystal piece directly. It converts it into a shape number relative to [`CliffSet`](/keys/cliffset/) and then indexes a forty-entry table of placement offsets with it. The read stays inside that table only while the crystal cliff set begins no more than thirty-six tiles after the cliff set. A theater that separates them further reads past the end of that table during random map generation. Nothing checks the index: the shape's placement offset then comes from whatever follows the table, and the piece is placed at that offset.
:::
