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

A theater's control file is its [`Root`](/keys/root/) plus `.INI`: `TEMPERAT.INI` for the temperate theater and `SNOW.INI` for snow. The game reads it each time a scenario starts or a saved game loads, after mounting that theater's archives, so the file can ship inside them. Each read replaces the tile sets read before. A loose copy is used ahead of an archived one; [OPENTS.INI](/formats/opents-ini/#the-order-files-are-searched-for-in) lists the folders searched and their order.

`[General]` assigns tile sets to the roles the engine uses them for: clear ground, water, cliffs, the height ramp base, and the rest. A role's value is a tile-set number, the `NNNN` of a `[TileSetNNNN]` section, not a tile index. The bridge piece keys, such as [`BridgeTopLeft1`](/keys/bridgetopleft1/), are the exception: their values are positions within a bridge set. A role that names a set the game does not read stays unset.

Tile sets are read in order from `[TileSet0000]` upward, with the number zero-padded to four digits. A theater may declare any number of sets.

:::caution[Number tile sets without gaps]
Reading stops at the first set number whose section has no [`TilesInSet`](/keys/tilesinset/) value, whether the section or only the key is missing. Sets numbered above that point are not read: their tiles are not part of the theater, and a `[General]` role naming one of them stays unset. `TilesInSet=-1` or any other negative value also ends reading at that set, and a negative value other than `-1` is written to the [debug log](/using/debug-logging/).
:::

A set's tile files are named from its [`FileName`](/keys/filename/), a two-digit tile number starting at `01`, and the theater's [`Suffix`](/keys/suffix/#scope-theater) as the extension. `FileName=RVCLIF` with `TilesInSet=8` loads `RVCLIF01.TEM` through `RVCLIF08.TEM` in the temperate theater, and `RVCLIF01.SNO` through `RVCLIF08.SNO` in snow.

A tile can have alternate artwork in files with a lowercase letter after the tile number, such as `RVCLIF01a.TEM` and `RVCLIF01b.TEM`. The game looks for the letters in order from `a` and stops at the first missing file, so `RVCLIF01c.TEM` is not used when `RVCLIF01b.TEM` is missing.

When a tile file with the theater's `Suffix` is missing, the game tries the same name with the theater's [`MMSuffix`](/keys/mmsuffix/): `.MMT` in the temperate theater and `.MMS` in snow. A set with [`NonMarbleMadness=0`](/keys/nonmarblemadness/) skips this second try.

A set can also have a section of per-tile animations, the section whose heading is the set's [`SetName`](/keys/setname/) value. `Tile<NN>Anim` names the animation for tile `NN` of the set, with `NN` matching the tile's file number. `Tile<NN>XOffset`, `Tile<NN>YOffset`, `Tile<NN>AttachesTo`, and `Tile<NN>ZAdjust` are read only for a tile whose `Tile<NN>Anim` names an animation. Alternate artwork never has an animation.

The animation that `Tile<NN>Anim` specifies is looked up among the animations the scenario's rules define. A name that the rules' `[Animations]` list leaves out gets a new animation type of that name.

```ini title="TEMPERAT.INI"
[General]
ClearTile=0     ; example tile-set numbers, not tile indices
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

The example leaves out sets `0000` through `0630`, which must all be present for `[TileSet0631]` to be read.
