---
key: Suffix
scope: theater
label: Theater artwork extension
see_also: [MMSuffix, Root, IsoRoot]
when_omitted:
  kind: context-dependent
  note: "`TEM` for TEMPERATE and `SNO` for SNOW, which keep their original settings; empty for any other theater, which then looks for artwork under no extension at all and has no unit remap palette."
---

The suffix is the file extension every piece of theater-specific artwork carries, and the stem of several files besides. A type marked [`Theater=yes`](/keys/theater/) loads `<name>.<Suffix>`, and so does each tile of a [tile set](/formats/theater-control/).

The same value names five more files:

| File | What it holds |
| --- | --- |
| `<Suffix>.MIX` | The theater's second archive |
| `ISO<Suffix>.PAL` | The palette the tiles are drawn through |
| `UNIT<Suffix>.PAL` | The palette unit and structure color schemes are built from |
| `SLOP01Z.<Suffix>` through `SLOP04Z.<Suffix>` | The depth shapes for sloped ground |
| `VEINHOLE.<Suffix>` | The veinhole monster's artwork |

```ini title="rules.ini"
[DESERT]
Suffix=DES      ; GACNST.DES, DES.MIX, ISODES.PAL, UNITDES.PAL
```

The three characters the original game used are not a limit; a longer suffix works everywhere the value is used.
