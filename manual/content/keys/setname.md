---
key: SetName
summary: Section heading a tile set's per-tile animations are read from, and the stem of the name each of its tiles uses.
see_also: [FileName, TilesInSet]
when_omitted:
  kind: value
  value: No Name
---

A section headed with the value, if the file has one, supplies the animation attached to each base tile of the set. [Theater control files](/formats/theater-control/) covers the entries that section accepts. The lookup is by exact section heading, so two sets sharing a value share the animation section, and each set's own tile numbering is applied to it.

```ini title="TEMPERAT.INI"
[TileSet0631]         ; example set
SetName=Riverbank cliffs
FileName=RVCLIF
TilesInSet=8

[Riverbank cliffs]    ; matched by name, not by set number
Tile03Anim=MYFALLS    ; example AnimType registered in rules.ini
```

Every tile the set produces also keeps a name of its own: the first 28 characters of the value with the tile's two-digit position appended. The set above yields `Riverbank cliffs 01` and upward. Nothing in the running game reads that name back; it is the section lookup that has an effect.

:::caution[Sets that omit the value share one animation section]
Every set without this key is looked up under the same heading, so a file that happens to contain a section with that name hands its entries to all of them at once.
:::
