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
  - { section: Theaters, id_from: value, entry_section: "<Theater ID>" }
  - { section: SuperWeaponTypes, id_from: value, entry_section: "<SuperWeaponType ID>" }
  - { section: BuildingTypes, id_from: value, entry_section: "<BuildingType ID>" }
  - { section: TerrainTypes, id_from: value, entry_section: "<TerrainType ID>" }
  - { section: SmudgeTypes, id_from: value, entry_section: "<SmudgeType ID>" }
  - { section: OverlayTypes, id_from: value, entry_section: "<OverlayType ID>" }
  - { section: Animations, id_from: value, entry_section: "<AnimType ID>" }
  - { section: VoxelAnims, id_from: value, entry_section: "<VoxelAnimType ID>" }
  - { section: Weapons, id_from: value, entry_section: "<WeaponType ID>" }
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

The two exceptions read their keys. In `[Sides]` the key is the Side ID and the value is a comma-separated HouseType list; a name in that list that is not a HouseType ID is logged and skipped, so a side does not stand for its countries there. In `[Tiberiums]` the key is a slot number: a number below the count already registered selects that existing Tiberium and the value is discarded, and only a number at or above it creates a new one under the name the value gives.

Registering an ID and defining it are separate passes. Registration creates the type carrying the built-in defaults for its kind, and the section named by the ID is read afterwards, so an ID registered with no section of its own is kept with those defaults rather than dropped.

OpenTS processes the selected `RULE*.INI`, then `LANGRULE.INI`, then `FIRESTRM.INI` when Firestorm is enabled, and finally `LANGFS.INI` when present. [Game data](/using/game-data/) covers what makes Firestorm count as installed.

`[Theaters]` is read on different terms from the rest, because a map names its theater before anything else about it is read and the archives that theater mounts are what the rest of the load comes out of. It is read once as the game starts, from the selected `RULE*.INI` and from `FIRESTRM.INI` whenever that file is installed rather than only when its addon is enabled — a theater's position must not move between one game and the next, because that position is the number maps, saves and the multiplayer checksum all carry. A map's own rules cannot add a theater.

Where no rules file declares the section, the two theaters Tiberian Sun shipped are registered in their original order, which is what every unmodified rules file gets. Where the section is present it is the whole roster, so it may drop `SNOW`, reorder the pair, or replace both — and a roster meaning to keep them has to write them out. Naming `TEMPERATE` or `SNOW` in the list does not create a second copy of it; the theater starts from its original settings and its own section overrides what it names.

```ini title="rules.ini"
[InfantryTypes]
0=MYINF

[MYINF]
Name=Example infantry
Strength=100
```

Projectiles have no registration section of their own. A projectile is created the first time a weapon's [`Projectile=`](/keys/projectile/) names it, then filled in from the section carrying its name on the same terms as above.

A weapon `[Weapons]` leaves out is created the same way, the first time a [`Primary=`](/keys/primary/), [`Secondary=`](/keys/secondary/), [`Elite=`](/keys/elite/), [`WeaponType=`](/keys/weapontype/), [`DropPodWeapon=`](/keys/droppodweapon/) or [`AirburstWeapon=`](/keys/airburstweapon/) names it. Weapon sections are read in one pass over the weapons registered by then, and a projectile's `AirburstWeapon=` is read after that pass, so a weapon only that key names is created too late to read its own section and keeps the built-in defaults.
