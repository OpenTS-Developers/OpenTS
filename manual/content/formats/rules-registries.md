---
format_id: rules-registries
title: Rules registration lists
summary: Lists in the rules files that register each kind of rules type and assign countries to sides.
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

Every registration section except `[Sides]` lists one ID per line, written as the value. The key only labels the line, but a repeated key drops the earlier line, as described below. The ID also names the section that defines the type:

```ini title="rules.ini"
[InfantryTypes]
0=MYINF

[MYINF]
Name=Example infantry
Strength=100
```

This registers an infantry type `MYINF` and reads its settings from `[MYINF]`.

Types are registered in the order their lines appear, and each rules file adds to the types that earlier files registered. The rules files are read in the order [Multiplayer rules](/formats/multiplayer-rules/#when-they-are-read) lists, and the scenario's own overrides come last.

A line registers nothing in these cases:

- Its value names an ID that is already registered, by an earlier line or an earlier file. The existing type keeps its place.
- Its value is empty, `none` or `<none>`.
- Another line later in the same section uses the same key. Only the later line's ID is registered, at the later line's place. This also applies when the section heading appears twice in one file.

Keep each ID to 24 characters or fewer. A longer ID is stored cut to its first 24 characters, and its settings are read from the section named by that shortened ID. A later reference that spells out the full ID does not find the stored type and creates another one.

Registering an ID creates the type with the built-in defaults for its kind, and its section is read afterwards. An ID with no section of its own is still registered and keeps those defaults.

`[Sides]` uses its keys. The key is the Side ID, and the value lists the countries on that side as HouseType IDs separated by commas. Each country must be registered by the same rules file or an earlier one. Write the list without spaces after the commas: a name that matches no registered country, including one with a leading space, is skipped, and that country does not join the side.

A later file that lists an existing side replaces that side's country list, so repeat every country the side should keep. A country left out of the new list keeps the side it had.

`[Tiberiums]` registers at most four types, as [Tiberium types](/systems/tiberium/#tiberium-types) explains.

## Theaters

`[Theaters]` is read once, as the game starts, and only from `RULES.INI` and from `FIRESTRM.INI`. `FIRESTRM.INI` counts whenever it is installed, even when Firestorm is not enabled; [Game data](/using/game-data/) covers what makes it count as installed. A map, the language rules files and the multiplayer rules files cannot add a theater. Saves record each theater by its position in the list, so the list has to stay the same from one game to the next.

When neither file registers a theater, the game uses the two theaters Tiberian Sun shipped, `TEMPERATE` and `SNOW`, in that order. Unmodified rules files get this list. A `[Theaters]` list that registers at least one theater replaces those two completely: it may drop `SNOW`, reorder the pair, or replace both, so a list that means to keep them has to name them.

A listed `TEMPERATE` or `SNOW` starts from that theater's original settings, and its section in the rules overrides them. Naming a theater again, later in the list or in `FIRESTRM.INI`, reuses the one already registered.

## Types named by other keys

Projectiles have no registration section. A projectile is created the first time a key names it, usually a weapon's [`Projectile=`](/keys/projectile/). A projectile a weapon names reads its settings from the section matching its name, as for a registered type.

A weapon that `[Weapons]` does not list is created the same way, the first time one of these keys names it: [`Primary=`](/keys/primary/), [`Secondary=`](/keys/secondary/), [`Elite=`](/keys/elite/), [`WeaponType=`](/keys/weapontype/), [`DropPodWeapon=`](/keys/droppodweapon/) or [`AirburstWeapon=`](/keys/airburstweapon/).

Register in `[Weapons]` any weapon that only a projectile's `AirburstWeapon=` names. Each rules file reads its weapon sections in one pass, and `AirburstWeapon=` is read after that pass. A weapon it creates therefore misses its section in that file, and keeps the built-in defaults unless a later rules file contains its section.
