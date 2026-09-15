---
format_id: theater-control
title: Theater control files
summary: Defines theater-wide tile-set indices and the properties of each tile set.
kind: file
filenames:
  - "<Theater root>.INI"
key_scopes:
  - file: theater control file
source_files:
  - code/isotype.cpp
  - code/_theater.cpp
  - code/theater.h
---

Each theater sets a [`Root`](/keys/root/), and its control file is that root plus `.INI`: `TEMPERAT.INI` for the temperate theater and `SNOW.INI` for snow. The file is read out of the theater archives. They are mounted with the theater palette once the map's `[Map] Theater=` value is known, and only when that value differs from the theater already loaded. A scenario that reuses the current theater therefore keeps the tile types it already holds. Reading the file discards the previous theater's tile types and loads the shared cell-shadow shape and the theater's own slope-depth shapes. Individual tile artwork is loaded afterwards, on demand, for the tiles a map actually places.

`[General]` assigns tile sets to engine roles: the clear ground, the water set, the cliff set, the height ramp base, and the rest. Its values are tile-set numbers (the `NNNN` of a `[TileSetNNNN]` section), not tile indices. Each is resolved to the index of that set's first tile during the pass that reads the numbered sections, so a role naming a set that pass never reaches stays unresolved.

Numbered sections are read in order from `[TileSet0000]` upward, with the number zero-padded to four digits.

:::caution[A numbering gap truncates the theater]
Reading stops at the first numbered section that has no [`TilesInSet`](/keys/tilesinset/), not at the highest number present. A set appended above a gap is never read, its tiles never join the theater's list, and a `[General]` role naming the gap or a number above it stays unresolved. A negative value other than `-1` is reported to the [debug log](/using/debug-logging/) and ends the same contiguous load.
:::

The number of tile sets a theater may declare is not capped. A set's [`MarbleMadness`](/keys/marblemadness/) and [`NonMarbleMadness`](/keys/nonmarblemadness/) values name another numbered set. A reference to a set the read never reached, or to a tile the target set does not have, is reported rather than taking the next set's.

Artwork for a set is named from its [`FileName`](/keys/filename/) plus a two-digit index starting at `01`, extended with the theater's [`Suffix`](/keys/suffix/). `FileName=RVCLIF` with `TilesInSet=8` loads `RVCLIF01.TEM` through `RVCLIF08.TEM` in the temperate theater, and the `.SNO` equivalents in snow. A trailing lowercase letter marks an alternate for the same tile, so `RVCLIF01a` and `RVCLIF01b` are further forms of `RVCLIF01`. Reading alternates stops at the first letter with no file. When a theater file is missing, the loader retries the same name with the theater's [`MMSuffix`](/keys/mmsuffix/), `.MMT` in temperate and `.MMS` in snow. The retry is skipped when the set sets [`NonMarbleMadness=0`](/keys/nonmarblemadness/).

A third kind of section is optional: one named by a set's own [`SetName`](/keys/setname/) value supplies per-tile animation for the set's base tiles. Lettered alternates never have one. `Tile<NN>Anim` names the animation for tile `NN` of that set, and `Tile<NN>XOffset`, `Tile<NN>YOffset`, `Tile<NN>AttachesTo`, and `Tile<NN>ZAdjust` are read once that animation resolves.

```ini title="TEMPERAT.INI"
[General]
ClearTile=0     ; tile-set numbers, not tile indices
WaterSet=21
CliffSet=10
ClearToRoughLat=14

[TileSet0631]      ; example set; 0631 is the number a [General] role would name
SetName=Riverbank cliffs
FileName=RVCLIF    ; loads RVCLIF01.TEM through RVCLIF08.TEM
TilesInSet=8
Morphable=no
AllowToPlace=yes
AllowTiberium=no

[Riverbank cliffs] ; named by this set's SetName
Tile03Anim=MYFALLS ; AnimType registered in rules.ini
Tile03ZAdjust=-12
```

The example shows one set of the many a theater file holds; the rest are left out.
