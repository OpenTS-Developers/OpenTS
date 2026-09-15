---
format_id: scenario-objects
title: Scenario object records
summary: Defines the vehicles, infantry, aircraft, structures, and trigger ownership loaded with a scenario.
kind: file
source_files:
- code/unit.cpp
- code/infantry.cpp
- code/aircraft.cpp
- code/building.cpp
- code/trigtype.cpp
- code/tagtype.cpp
- code/house.cpp
- code/scenario.cpp
filenames:
- "<scenario>.INI"
- "*.MAP"
- "*.MPR"
related:
- type: format
  id: ini-syntax
- type: system
  id: starting-forces
- type: key
  id: House
- type: key
  id: Allies
- type: key
  id: NodeCount
- type: key
  id: UseMPAIBaseNodes
---

Scenario INI files store placed objects as comma-separated entries in `[Units]`, `[Infantry]`, `[Aircraft]`, and `[Structures]`. Every row begins with an owner, an ObjectType ID, and strength; the remaining positional fields hold the location, facing, mission, tag, and type-specific state. The engine writes each section entry name as that row's own number, while the loader ignores the name and processes rows in their section order.

## Runtime owners

A HouseType definition and a live house are different things. Rules can register a HouseType that the current campaign or multiplayer session never instantiates. A vehicle, infantry, aircraft, or structure row is created only when its first field names a registered HouseType with a live house in the current session, or a [spawn house](#spawn-houses) whose position somebody holds. The row is skipped before an object is allocated when its owner is unknown or has no live house. The type, location, and remaining fields are not used. Other validation, including whether the object can be placed at its requested location, still applies afterwards, and an object that cannot be placed is deleted. Every machine in a multiplayer game creates the same rows, the ones owned by the house it plays included.

Trigger definitions in `[Triggers]` use the same live-owner rule, and a trigger is bound to the live house it resolved to as the scenario loads. The legacy `<none>` owner selects the first house type the rules register and is accepted only when that house has a live instance. Trigger names are registered before their bodies are read so links may point forward, but a definition whose owner does not resolve is deleted. A link naming a missing or rejected definition remains empty, and a `[Tags]` row naming one receives no trigger and remains inert. Owner names resolve only through registered HouseTypes, the houses in the session, and the spawn houses below.

## Spawn houses

A spawn house names whoever starts at one of the eight numbered start positions, waypoints `0` through `7`. It is written `Spawn1` through `Spawn8` or `<Player @ A>` through `<Player @ H>`. Both spellings mean the same position, are matched without regard to case, and must be spelled exactly, so `Spawn 1` and `Spawn9` name nothing. No HouseType is registered under these names and a scenario cannot define one. Which house holds each position is settled as the scenario loads, before any team, trigger, or object row is read. A spawn owner therefore resolves to the house that starts there. [Starting forces](/systems/starting-forces/#the-start-position) owns how positions are handed out. An observer and a house of a passive country never hold one, and a campaign never assigns any.

A vehicle, infantry, aircraft, or structure row owned by a spawn house is created for that house. A spawn house nobody holds, such as `Spawn3` in a two-player game, resolves to nothing. The row is skipped like any other row without a live owner. A `[Triggers]` definition owned by it is deleted, and a [TeamType](/keys/house/) owned by it raises no team. Each owner dropped this way is written to the debug log.

A spawn house may also have a section of its own, `[Spawn1]` through `[Spawn8]`, read for the house holding that position once positions are settled. It sets [`Allies=`](/keys/allies/) in every skirmish or multiplayer game, and the base node keys, [`NodeCount`](/keys/nodecount/) with its numbered entries, when the map sets [`UseMPAIBaseNodes=yes`](/keys/usempaibasenodes/). A section for a position nobody holds is ignored, and the engine does not write these sections back.

Trigger event, trigger action, and team mission parameters that take a house by number accept `50` through `57`, or `4475` through `4482`, as `Spawn1` through `Spawn8`. Any other number that is not the index of a registered HouseType names nothing, `58` through `60` included. An event that needs the house never trips, and an action or mission that needs it does nothing. When the engine writes a scenario, a spawn owner is written as the country of the house it resolved to.

## Vehicle follower IDs

The follower field, which follows the flag marking a `[Units]` row as standing on a bridge, is the zero-based source-row position of the vehicle that follows it, or `-1` for no follower. OpenTS resolves these links only after every unit row has been considered. Each successfully placed vehicle remains mapped to its original section position, so rejecting an earlier row does not renumber the rows after it. A follower position that is negative, outside the section, skipped for its owner or type, or rejected during placement produces no link. It is never redirected to whichever vehicle happened to occupy the compacted runtime list position.
