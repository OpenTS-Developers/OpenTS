---
key: Selectable
scope: aircrafttype
label: Player selection
when_omitted:
  kind: context-dependent
  note: An AircraftType, BuildingType, InfantryType, UnitType, ParticleType, ParticleSystemType or VoxelAnimType section starts at yes. A BulletType, OverlayType, SmudgeType or TerrainType section starts at no.
---

Clearing the flag closes every route into a selection. The cursor over the object never becomes the select cursor, and a click on it selects nothing. A band box drawn across it passes it by, and cycling forward or backward through the army steps over it. The power-toggle cursor also refuses it.

```ini title="rules.ini"
[MYORCA] ; example AircraftType
Selectable=no
```
