---
format_id: rules-registries
title: Rules registration lists
summary: Registers named rules types and Side membership from rules-layer lists.
kind: registry
files:
  - RULE*.INI
  - LANGRULE.INI
  - FIRESTRM.INI
  - LANGFS.INI
registrations:
  - { section: InfantryTypes, id_from: value, entry_section: "<InfantryType ID>" }
  - { section: Houses, id_from: value, entry_section: "<HouseType ID>" }
  - { section: VehicleTypes, id_from: value, entry_section: "<UnitType ID>" }
  - { section: AircraftTypes, id_from: value, entry_section: "<AircraftType ID>" }
  - { section: Sides, id_from: key, value: "Comma-separated HouseType IDs" }
  - { section: SuperWeaponTypes, id_from: value, entry_section: "<SuperWeaponType ID>" }
  - { section: BuildingTypes, id_from: value, entry_section: "<BuildingType ID>" }
  - { section: TerrainTypes, id_from: value, entry_section: "<TerrainType ID>" }
  - { section: SmudgeTypes, id_from: value, entry_section: "<SmudgeType ID>" }
  - { section: OverlayTypes, id_from: value, entry_section: "<OverlayType ID>" }
  - { section: Animations, id_from: value, entry_section: "<AnimType ID>" }
  - { section: VoxelAnims, id_from: value, entry_section: "<VoxelAnimType ID>" }
  - { section: Warheads, id_from: value, entry_section: "<WarheadType ID>" }
  - { section: Particles, id_from: value, entry_section: "<ParticleType ID>" }
  - { section: ParticleSystems, id_from: value, entry_section: "<ParticleSystemType ID>" }
  - { section: Tiberiums, id_from: value, entry_section: "<Tiberium ID>" }
source_files:
  - code/rules.cpp
  - code/tiberium.cpp
  - code/init.cpp
---

All but two of these registration sections are read the same way. Each entry is taken by its position in the section and only its value is looked at, so the key text decides nothing and the order the lines are written in is the order the types are registered. The value is both the type ID and the name of the section the definition is written in, and is kept to its first thirty-one characters. An empty value registers nothing, and a value naming an ID the game already carries reuses that type instead of adding a second one.

The two exceptions read their keys. In `[Sides]` the key is the Side ID and the value is a comma-separated HouseType list. In `[Tiberiums]` the key is a slot number: a number below the count already registered selects that existing Tiberium and the value is discarded, and only a number at or above it creates a new one under the name the value gives.

Registering an ID and defining it are separate passes. Registration creates the type carrying the built-in defaults for its kind, and the section named by the ID is read afterwards, so an ID registered with no section of its own is kept with those defaults rather than dropped.

OpenTS processes the selected `RULE*.INI`, then `LANGRULE.INI`, then `FIRESTRM.INI` when Firestorm is enabled, and finally `LANGFS.INI` when present. [Game data](/using/game-data/) covers what makes Firestorm count as installed.

```ini title="rules.ini"
[InfantryTypes]
0=MYINF

[MYINF]
Name=Example infantry
Strength=100
```

Weapons and projectiles have no registration section of their own. A weapon is created the first time a type's [`Primary=`](/keys/primary/), [`Secondary=`](/keys/secondary/) or [`Elite=`](/keys/elite/) names it, and a projectile the first time a weapon's [`Projectile=`](/keys/projectile/) names it. Each is then filled in from the section carrying its name on the same terms as above, and keeps the built-in defaults where no section carries it.
