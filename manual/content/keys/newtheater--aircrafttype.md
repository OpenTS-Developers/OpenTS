---
key: NewTheater
scope: aircrafttype
label: Theater artwork naming
see_also: ["Theater", "Image", "Voxel"]
when_omitted:
  kind: value
  value: "no"
---

On an AircraftType this flag does nothing. A voxel aircraft has no shape file for the rewrite to touch. A shape aircraft fetches its plain `<Image ID>.SHP` once more, immediately after the rewrite, and keeps whatever that fetch returns, so the rewritten name never survives the read. Marking [`Theater=yes`](/keys/theater/) as well changes nothing here.

```ini title="art.ini"
[APACHE] ; the Image ID of an AircraftType
NewTheater=yes ; no effect: the aircraft keeps drawing APACHE.SHP
```

No stock aircraft art section carries this flag or [`Theater`](/keys/theater/); every shipped aircraft section sets [`Voxel=yes`](/keys/voxel/).

On other scopes the flag renames real shape files. The file keeps its `.SHP` extension, and the second letter of its name is rewritten to the theater's [`ImageLetter`](/keys/imageletter/) instead: `T` in temperate and `A` in snow. The rewrite happens only where that second letter is already the image letter of some declared theater, and the comparison ignores case. Any other name is left exactly as written. The shipped civilian artwork sets the flag and gains nothing from it. `CITY01` through `CITY22`, `ABAN01` through `ABAN18` and `BBOARD01` through `BBOARD16` set it in `art.ini`, and `MWAR`, its `_A` through `_C` variants, and `OBL1` carry it in the Firestorm art file `artfs.ini`. Their second letters match no theater's image letter.

```ini title="art.ini"
[GACNST] ; the Image ID of a BuildingType
NewTheater=yes ; draws GTCNST.SHP in temperate and GACNST.SHP in snow
```

A [`Voxel=yes`](/keys/voxel/) InfantryType or UnitType is in the same position as an aircraft: no shape to rename. A BuildingType resolves and loads a theater-named shape either way.

:::caution[Only some of these types keep the rewrite]
An AircraftType, InfantryType, or UnitType looks its unrewritten `<Image ID>.SHP` up again immediately after the rename and keeps whatever that returns. Those three, a BulletType, a ParticleType, a ParticleSystemType and a VoxelAnimType also leave their artwork as it stands when the scenario theater changes. A SmudgeType and a TerrainType do too: their theater pass handles [`Theater`](/keys/theater/) alone.
:::

:::note[A building is renamed whether or not the flag is set]
A BuildingType runs its shape, its buildup, and its door, deploy, bib, and Z-overlay animations through the same rewrite every time its rules section is read. The flag is what makes a building resolve those names again against a new theater.
:::
