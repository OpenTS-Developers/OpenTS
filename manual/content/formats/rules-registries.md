---
format_id: rules-registries
title: Rules registration lists
summary: Registers named rules types and Side membership from rules-layer lists.
kind: registry
files:
  - RULES.INI
  - LANGRULE.INI
  - FIRESTRM.INI
  - LANGFS.INI
  - MPLAYER.INI
  - MPLAYERFS.INI
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

Every one of these registration sections except `[Sides]` is read the same way. Each entry is taken by its position in the section, and only its value is looked at. The key text decides nothing, and the order the lines are written in is the order the types are registered. The value is both the type ID and the name of the section the definition is written in, and is kept to its first thirty-one characters. An empty value registers nothing: the line is dropped, so the entries written after it each move up one position. A value naming an ID the game already has reuses that type instead of adding a second one.

`[Sides]` reads its keys instead. The key is the Side ID and the value is a comma-separated HouseType list. A name in that list that is not a HouseType ID is logged and skipped, so the country it names does not join that side. `[Tiberiums]` stops registering new types at four, as [Tiberium types](/systems/tiberium/#tiberium-types) explains.

Registering an ID and defining it are separate passes. Registration creates the type with the built-in defaults for its kind, and the section named by the ID is read afterwards. An ID registered with no section of its own is therefore kept with those defaults rather than dropped.

OpenTS processes `RULES.INI`, then `LANGRULE.INI`, then `FIRESTRM.INI` when Firestorm is enabled, then `LANGFS.INI` when present. Outside a campaign it then processes [`MPLAYER.INI`](/formats/multiplayer-rules/), and `MPLAYERFS.INI` as well when Firestorm is enabled. The scenario's own overrides come after all of them. [Game data](/using/game-data/) covers what makes Firestorm count as installed.

`[Theaters]` is read differently from the rest. A map names its theater before anything else about it is read, and the archives that theater mounts supply the rest of the load. The roster therefore has to be settled before any map is loaded. It is read once as the game starts, from `RULES.INI` and from `FIRESTRM.INI` whenever that file is installed, not only when its addon is enabled. A theater's position must not move between one game and the next, and that position is the number maps, saves and the multiplayer checksum all use. Neither a map's own rules nor the multiplayer files can add a theater, for the same reason: both are read per game, and a roster that changed with the game type would move every theater after the one it added.

Where no rules file declares the section, the two theaters Tiberian Sun shipped are registered in their original order, which is what every unmodified rules file gets. Where the section is present it is the whole roster: it may drop `SNOW`, reorder the pair, or replace both. A roster that means to keep them has to write them out. Naming `TEMPERATE` or `SNOW` in the list does not create a second copy of that theater. It starts from the original settings, and its own section overrides what it names.

```ini title="rules.ini"
[InfantryTypes]
0=MYINF

[MYINF]
Name=Example infantry
Strength=100
```

Projectiles have no registration section of their own. A projectile is created the first time a weapon's [`Projectile=`](/keys/projectile/) names it, then filled in from the section matching its name on the same terms as above.

A weapon `[Weapons]` leaves out is created the same way, the first time a key outside `[Weapons]` names it. The keys that can do this are [`Primary=`](/keys/primary/), [`Secondary=`](/keys/secondary/), [`Elite=`](/keys/elite/), [`WeaponType=`](/keys/weapontype/), [`DropPodWeapon=`](/keys/droppodweapon/) and [`AirburstWeapon=`](/keys/airburstweapon/). Weapon sections are read in one pass over the weapons registered by then. A projectile's `AirburstWeapon=` is read after that pass, so a weapon only that key names is created too late to read its own section and keeps the built-in defaults.
