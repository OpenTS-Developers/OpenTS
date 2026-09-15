---
enum_id: RTTIType
slug: rtti-type
title: Runtime object type
summary: Engine object-kind tokens accepted by INI settings such as `Factory=`.
representation: token
bindings:
  key_value_types: [RTTIType]
  scripting_parameter_types: []
source_files: [code/rtti.hh, code/_rtti.cpp]
values:
  - { constant: RTTI_NONE, value: 0, input: "<none>", meaning: "No runtime type." }
  - { constant: RTTI_UNIT, value: 1, input: "Unit", meaning: "Live vehicle object." }
  - { constant: RTTI_AIRCRAFT, value: 2, input: "Aircraft", meaning: "Live aircraft object." }
  - { constant: RTTI_AIRCRAFTTYPE, value: 3, input: "AircraftType", meaning: "Aircraft type definition." }
  - { constant: RTTI_ANIM, value: 4, input: "Anim", meaning: "Live animation object." }
  - { constant: RTTI_ANIMTYPE, value: 5, input: "AnimType", meaning: "Animation type definition." }
  - { constant: RTTI_BUILDING, value: 6, input: "Building", meaning: "Live building object." }
  - { constant: RTTI_BUILDINGTYPE, value: 7, input: "BuildingType", meaning: "Building type definition." }
  - { constant: RTTI_BULLET, value: 8, input: "Bullet", meaning: "Live projectile object." }
  - { constant: RTTI_BULLETTYPE, value: 9, input: "BulletType", meaning: "Projectile type definition." }
  - { constant: RTTI_CAMPAIGN, value: 10, input: "Campaign", meaning: "Campaign definition." }
  - { constant: RTTI_CELL, value: 11, input: "Cell", meaning: "Map cell object." }
  - { constant: RTTI_FACTORY, value: 12, input: "Factory", meaning: "Production factory object." }
  - { constant: RTTI_HOUSE, value: 13, input: "House", meaning: "Live player or house object." }
  - { constant: RTTI_HOUSETYPE, value: 14, input: "HouseType", meaning: "House type definition." }
  - { constant: RTTI_INFANTRY, value: 15, input: "Infantry", meaning: "Live infantry object." }
  - { constant: RTTI_INFANTRYTYPE, value: 16, input: "InfantryType", meaning: "Infantry type definition." }
  - { constant: RTTI_ISOTILE, value: 17, input: "Isotile", meaning: "Live isometric tile object." }
  - { constant: RTTI_ISOTILETYPE, value: 18, input: "IsotileType", meaning: "Isometric tile type definition." }
  - { constant: RTTI_LIGHT, value: 19, input: "Light", meaning: "A structure's sweeping searchlight beam." }
  - { constant: RTTI_OVERLAY, value: 20, input: "Overlay", meaning: "Live overlay object." }
  - { constant: RTTI_OVERLAYTYPE, value: 21, input: "OverlayType", meaning: "Overlay type definition." }
  - { constant: RTTI_PARTICLE, value: 22, input: "Particle", meaning: "Live particle object." }
  - { constant: RTTI_PARTICLETYPE, value: 23, input: "ParticleType", meaning: "Particle type definition." }
  - { constant: RTTI_PARTICLESYSTEM, value: 24, input: "ParticleSystem", meaning: "Live particle-system object." }
  - { constant: RTTI_PARTICLESYSTEMTYPE, value: 25, input: "ParticleSystemType", meaning: "Particle-system type definition." }
  - { constant: RTTI_SCRIPT, value: 26, input: "Script", meaning: "Live team script object." }
  - { constant: RTTI_SCRIPTTYPE, value: 27, input: "ScriptType", meaning: "Team script type definition." }
  - { constant: RTTI_SIDE, value: 28, input: "Side", meaning: "Side definition." }
  - { constant: RTTI_SMUDGE, value: 29, input: "Smudge", meaning: "Live smudge object." }
  - { constant: RTTI_SMUDGETYPE, value: 30, input: "SmudgeType", meaning: "Smudge type definition." }
  - { constant: RTTI_SPECIAL, value: 31, input: "Special", meaning: "Not an object kind: marks a sidebar cameo whose identifier names a superweapon." }
  - { constant: RTTI_SUPERWEAPONTYPE, value: 32, input: "SuperWeaponType", meaning: "Superweapon type definition." }
  - { constant: RTTI_TASKFORCE, value: 33, input: "TaskForce", meaning: "Task-force definition." }
  - { constant: RTTI_TEAM, value: 34, input: "Team", meaning: "Live AI team object." }
  - { constant: RTTI_TEAMTYPE, value: 35, input: "TeamType", meaning: "AI team type definition." }
  - { constant: RTTI_TERRAIN, value: 36, input: "Terrain", meaning: "Live terrain object." }
  - { constant: RTTI_TERRAINTYPE, value: 37, input: "TerrainType", meaning: "Terrain type definition." }
  - { constant: RTTI_TRIGGER, value: 38, input: "Trigger", meaning: "Live scenario trigger object." }
  - { constant: RTTI_TRIGGERTYPE, value: 39, input: "TriggerType", meaning: "Trigger type definition." }
  - { constant: RTTI_UNITTYPE, value: 40, input: "UnitType", meaning: "Vehicle type definition." }
  - { constant: RTTI_VOXELANIM, value: 41, input: "VoxelAnim", meaning: "Live voxel animation object." }
  - { constant: RTTI_VOXELANIMTYPE, value: 42, input: "VoxelAnimType", meaning: "Voxel animation type definition." }
  - { constant: RTTI_WAVE, value: 43, input: "Wave", meaning: "A laser or sonic beam drawn between a firing object and its target." }
  - { constant: RTTI_TAG, value: 44, input: "Tag", meaning: "Live trigger tag object." }
  - { constant: RTTI_TAGTYPE, value: 45, input: "TagType", meaning: "Trigger tag type definition." }
  - { constant: RTTI_TIBERIUM, value: 46, input: "Tiberium", meaning: "Tiberium type definition." }
  - { constant: RTTI_ACTION, value: 47, input: "Action", meaning: "Trigger action record." }
  - { constant: RTTI_EVENT, value: 48, input: "Event", meaning: "Trigger event record." }
  - { constant: RTTI_WEAPONTYPE, value: 49, input: "WeaponType", meaning: "Weapon type definition." }
  - { constant: RTTI_WARHEADTYPE, value: 50, input: "WarheadType", meaning: "Warhead type definition." }
  - { constant: RTTI_WAYPOINT, value: 51, input: "Waypoint", meaning: "Scenario waypoint object." }
  - { constant: RTTI_ABSTRACT, value: 52, input: "Abstract", meaning: "Not an object kind: marks a stored target that names an object rather than a cell." }
  - { constant: RTTI_TUBE, value: 53, input: "Tube", meaning: "An underground tunnel route: its two mouths and the path of steps between them." }
  - { constant: RTTI_LIGHTSOURCE, value: 54, input: "LightSource", meaning: "An unseen point light that brightens and tints the cells around it." }
  - { constant: RTTI_EMPULSE, value: 55, input: "EMPulse", meaning: "EMPulse effect object." }
  - { constant: RTTI_TACTICALMAP, value: 56, input: "TacticalMap", meaning: "Tactical map object." }
  - { constant: RTTI_SUPERWEAPON, value: 57, input: "SuperWeapon", meaning: "Live superweapon object." }
  - { constant: RTTI_AITRIGGER, value: 58, input: "AITrigger", meaning: "Live AI trigger object." }
  - { constant: RTTI_AITRIGGERTYPE, value: 59, input: "AITriggerType", meaning: "AI trigger type definition." }
  - { constant: RTTI_NEURON, value: 60, input: "Neuron", meaning: "A node of an unfinished AI brain; nothing creates one while the game runs." }
  - { constant: RTTI_FOGGEDOBJECT, value: 61, input: "FoggedObject", meaning: "Fog-memory object." }
  - { constant: RTTI_ALPHASHAPE, value: 62, input: "AlphaShape", meaning: "The blend shape that brightens the map around an object with an AlphaImage." }
  - { constant: RTTI_VEINHOLEMONSTER, value: 63, input: "VeinholeMonster", meaning: "Veinhole monster object." }
---

These are engine object kinds rather than identifiers a rules file defines. A [`Factory`](/keys/factory/) assignment names one of them to say what kind of object a structure produces rather than the particular vehicle, structure or other definition. That page covers which of the kinds put a structure to work at all.

Most of the list never appears in an assignment. Every live object and every definition has a kind, so the engine can tell the two apart while the game runs. That is why so many names come in pairs: `Building` for a structure standing on the map, `BuildingType` for the rules entry it was made from. The rest name machinery a rules file has no use for: a cell, a production line, the record a fogged cell keeps of what stood in it, the tunnel route joining one tunnel mouth to another. A name matching nothing in the list resolves to `<none>` rather than leaving the stored value alone, so a misspelling is read as naming no kind.
