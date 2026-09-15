---
key: Theater
scope: aircrafttype
label: Theater-specific artwork
see_also: ["NewTheater", "Image", "Voxel"]
when_omitted:
  kind: value
  value: "no"
---

On an AircraftType this flag does nothing. A voxel aircraft has no shape file to rename. A shape aircraft fetches its plain `<Image ID>.SHP` once more, immediately after the extension swap, and keeps whatever that fetch returns, so the theater file never survives the read. Marking [`NewTheater=yes`](/keys/newtheater/) as well changes nothing here.

```ini title="art.ini"
[ORCA] ; the Image ID of an AircraftType
Theater=yes ; no effect: the aircraft keeps drawing ORCA.SHP
```

No stock aircraft art section carries this flag or [`NewTheater`](/keys/newtheater/); every shipped aircraft section sets [`Voxel=yes`](/keys/voxel/).

On other scopes the flag substitutes a theater-specific shape file. The shape file keeps the Image ID as its name and exchanges its extension for the theater's own: `.TEM` in temperate and `.SNO` in snow. Nothing else about the name changes. A BuildingType, OverlayType, SmudgeType or TerrainType marked with the flag loads the theater-named file alone: there is no fallback to `.SHP`, so a type whose theater file is missing is left with no shape.

```ini title="art.ini"
[MYROCK] ; the Image ID of a TerrainType
Theater=yes ; draws MYROCK.TEM or MYROCK.SNO, never MYROCK.SHP
```

A [`Voxel=yes`](/keys/voxel/) InfantryType or UnitType is in the same position as an aircraft: no shape to rename at all. A BuildingType resolves and loads a theater-named shape either way.

:::caution[Only some of these types keep the substitution]
An AircraftType, InfantryType, or UnitType looks its plain `<Image ID>.SHP` up again immediately after the flag has been applied and keeps whatever that returns. The theater file never reaches it. Those three, a BulletType, a ParticleType, a ParticleSystemType and a VoxelAnimType also leave their artwork as it stands when the scenario theater changes. A BuildingType, OverlayType, SmudgeType, or TerrainType marked with the flag resolves its name again against the new theater.
:::
