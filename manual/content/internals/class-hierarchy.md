---
title: Object and type system
summary: Where runtime state, shared type data, engine IDs, factories, and INI reads belong.
category: architecture
source_files:
  - code/abstract.h
  - code/abstract.cpp
  - code/abstype.h
  - code/abstype.cpp
  - code/object.h
  - code/object.cpp
  - code/objtype.h
  - code/objtype.cpp
  - code/mission.h
  - code/radio.h
  - code/techno.h
  - code/foot.h
  - code/aircraft.h
  - code/techtype.h
  - code/techtype.cpp
---

`AbstractClass` is the common base for persistent engine entities. Map objects and INI-backed type definitions are separate branches of that hierarchy. A runtime instance stores state for one object in the current match; a type definition stores data shared by every instance with the same INI identifier.

This page covers simulation objects and their definitions. UI controls, file classes, and locomotors use other hierarchies.

## Terms

| Term | Meaning in this manual |
| --- | --- |
| Runtime instance | One object or state record created for the current match, such as a `UnitClass` or `HouseClass`. |
| Type definition | Shared configuration loaded once for an INI identifier, such as a `UnitTypeClass`. |
| ObjectType ID | For an `ObjectTypeClass` family, the identifier returned by `Name()` and used as that definition's INI section name. Other `AbstractTypeClass` families can use `IniName` as an entry key in a registry section. |
| RTTI | The engine's `RTTIType` value. It is separate from C++ RTTI. |
| Heap ID | The index returned by `Fetch_Heap_ID()` for classes stored in a family-specific heap. |

`IsActive`, `IsInLimbo`, and `Strength` are independent runtime fields. The phrase "runtime instance" does not imply a particular value for any of them.

## Primary runtime hierarchy

The tree below shows the primary behavioral base path. Parenthesized bases are additional C++ bases, not part of that path.

```text title="Primary runtime inheritance"
AbstractClass
+- ObjectClass
   +- MissionClass
   |  +- RadioClass
   |     +- TechnoClass (+ FlasherClass, StageClass)
   |        +- BuildingClass
   |        +- FootClass
   |           +- AircraftClass (+ IFlyControl)
   |           +- InfantryClass
   |           +- UnitClass
   +- AnimClass (+ StageClass)
   +- BulletClass
   +- BuildingLightClass
   +- IsometricTileClass
   +- OverlayClass
   +- ParticleClass
   +- ParticleSystemClass
   +- SmudgeClass
   +- TerrainClass (+ StageClass)
   +- VeinholeMonsterClass
   +- VoxelAnimClass (+ BounceClass)
   +- WaveClass
```

`ObjectClass` supplies map position, strength, limbo and active state, cell links, tags, render-layer membership, and the `Class_Of()` interface. `MissionClass` adds mission state; `RadioClass` adds synchronous object-to-object coordination; `TechnoClass` adds ownership, combat, targeting, cargo, and production state. `FootClass` is the mobile techno base.

Secondary bases add narrow behavior. For example, `StageClass` supplies staged animation, while `IFlyControl` is the aircraft locomotion control interface. Code that follows only the primary path must not assume that these contracts are absent.

Other persistent state derives from `AbstractClass` without being a map object or a type definition. Important families include `HouseClass`, `TeamClass`, `TagClass`, `TriggerClass`, `TActionClass`, `TEventClass`, `ScriptClass`, `FactoryClass`, `CellClass`, and `SuperClass`.

## Type-definition hierarchy

`AbstractTypeClass` also derives from `AbstractClass`. Type definitions therefore use the same engine identity and persistence interfaces as runtime state, but they do not derive from `ObjectClass`.

```text title="INI-backed type definitions"
AbstractClass
+- AbstractTypeClass
   +- ObjectTypeClass
   |  +- TechnoTypeClass
   |  |  +- AircraftTypeClass
   |  |  +- BuildingTypeClass
   |  |  +- InfantryTypeClass
   |  |  +- UnitTypeClass
   |  +- AnimTypeClass
   |  +- BulletTypeClass
   |  +- IsometricTileTypeClass
   |  +- OverlayTypeClass
   |  +- ParticleSystemTypeClass
   |  +- ParticleTypeClass
   |  +- SmudgeTypeClass
   |  +- TerrainTypeClass
   |  +- VoxelAnimTypeClass
   +- AITriggerTypeClass
   +- CampaignClass
   +- HouseTypeClass
   +- ScriptTypeClass
   +- SideClass
   +- SuperWeaponTypeClass
   +- TagTypeClass
   +- TaskForceClass
   +- TeamTypeClass
   +- TiberiumClass
   +- TriggerTypeClass
   +- WarheadTypeClass
   +- WeaponTypeClass
```

`AbstractTypeClass` owns the INI identifier (`IniName`), display-name token (`GivenName`), and the common `Read_INI`/`Write_INI` contract. `ObjectTypeClass` adds properties shared by map-object definitions, including artwork identifiers, strength, armor, footprint behavior, and object factories. `TechnoTypeClass` adds data shared by owned combat actors, including weapons, movement, production, targeting, and veterancy.

Direct `AbstractTypeClass` families do not describe map objects. A weapon, warhead, team, trigger, or campaign definition therefore has no reason to inherit `ObjectTypeClass` just because it has an INI section.

## Identity, heaps, and lookup

`AbstractClass` exposes three different identifiers, and they are not interchangeable:

| Interface | Purpose |
| --- | --- |
| `What_Am_I()` / `Fetch_RTTI()` | Returns the concrete engine `RTTIType`. Abstract intermediate classes without an override remain abstract; they do not supply `RTTI_NONE` as a default identity. |
| `Fetch_ID()` | Returns the object's engine identity used by persistence and reference tracking. |
| `Fetch_Heap_ID()` | Returns the slot in a family-specific heap when the concrete class overrides it; the base implementation returns zero. |

An RTTI value selects a class family; a heap ID selects one object within that family's heap; an ObjectType ID is the textual INI name.

Most INI-backed `ObjectClass` families expose their shared definition through `Class_Of()`; techno instances also expose it through `Techno_Type_Class()`. This is not a universal retained pointer. `BuildingLightClass` and `WaveClass` return null, while `VeinholeMonsterClass` obtains its rules-owned type through global state.

The table names the interface for each direction of travel between a definition and a runtime instance. The first two rows are lookups; the last two create an object from a definition, and they are virtual hooks rather than guaranteed reverse mappings.

| Direction | Interface |
| --- | --- |
| Runtime instance to shared definition | `Class_Of()` or `Techno_Type_Class()` when the family provides one |
| Textual ID to object definition | `ObjectTypeClass::From_Name()` and family-specific lookup |
| Definition to unplaced instance | `Create_One_Of(HouseClass*)` |
| Definition to placed instance | `Create_And_Place(Cell, HouseClass*)` |

`AircraftTypeClass::Create_And_Place()` returns false, and current particle, particle-system, animation, bullet, and voxel-animation implementations include false, null, or no-op factory paths. Check the concrete family and the return value.

`ObjectTypeClass::ObjectTypes` is the cross-family registry of object definitions. Concrete families also use their own heaps and typed lookup functions. Code that already knows the family should use the typed path instead of scanning the cross-family registry.

## INI read order

Base-reader calls are an implementation property, not a consequence of C++ inheritance. A `UnitTypeClass` read passes through `AbstractTypeClass`, `ObjectTypeClass`, and `TechnoTypeClass` before applying unit-specific fields. `AITriggerTypeClass`, `TagTypeClass`, `TriggerTypeClass`, `WarheadTypeClass`, and `WeaponTypeClass` instead parse their fields without calling `AbstractTypeClass::Read_INI`. Trace the concrete reader before treating a base-class key as inherited.

The declaring reader determines where a key enters the hierarchy:

- `AbstractTypeClass::Read_INI` applies to every definition whose read path reaches it.
- `ObjectTypeClass::Read_INI` applies only to map-object definitions.
- `TechnoTypeClass::Read_INI` applies only to aircraft, buildings, infantry, and units.
- A concrete reader limits the key to that family unless another reader accepts the same spelling independently.

A derived reader may reinterpret or overwrite a field after its base call. The declaring class establishes the read location, not identical behavior in every derived family.

## Where new state belongs

Each row pairs a requirement with the class that should hold the field. The rows run from state that varies per object, through the definition classes that share it, out to state that belongs to the match, with a final row for a field that refers to another object. Take the narrowest class that owns the invariant: moving a field upward widens the serialized layout and public INI surface, and moving it downward duplicates state and reader logic.

| Requirement | Placement | Example |
| --- | --- | --- |
| Changes independently for every object during a match | The narrowest runtime class that owns the behavior | Remaining fuel on one `AircraftClass` instance |
| Shared by every object with one INI identifier | The corresponding concrete `TypeClass` | Maximum fuel on `AircraftTypeClass` |
| Shared by several map-object definition families | `ObjectTypeClass` or another common type base | A property used by animations, terrain, and technos |
| Shared only by owned combat definitions | `TechnoTypeClass` | A targeting coefficient used by units, infantry, aircraft, and buildings |
| Applies to the match rather than an object definition | The owning global or scenario subsystem | A game-wide rule in `RulesClass` |
| References another persistent runtime object | Runtime state plus detach, load swizzling, and CRC review | A non-owning target or contact pointer |
