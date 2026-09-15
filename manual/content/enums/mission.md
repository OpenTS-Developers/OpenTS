---
enum_id: MissionType
slug: mission
title: Mission
summary: Runtime mission state machines assignable through mission-valued scripting payloads.
representation: integer
bindings:
  key_value_types: []
  scripting_parameter_types: [mission]
source_files: [code/mission.hh, code/_mission.cpp]
values:
  - { constant: MISSION_SLEEP, value: 0, input: "0", meaning: "Sleep and perform no active task." }
  - { constant: MISSION_ATTACK, value: 1, input: "1", meaning: "Attack the nearest suitable enemy." }
  - { constant: MISSION_MOVE, value: 2, input: "2", meaning: "Move toward the assigned destination." }
  - { constant: MISSION_QMOVE, value: 3, input: "3", meaning: "Rewritten to Move before the mission is queued." }
  - { constant: MISSION_RETREAT, value: 4, input: "4", meaning: "Retreat toward home or safety." }
  - { constant: MISSION_GUARD, value: 5, input: "5", meaning: "Hold position and guard." }
  - { constant: MISSION_STICKY, value: 6, input: "6", meaning: "Hold position without being recruited." }
  - { constant: MISSION_ENTER, value: 7, input: "7", meaning: "Enter another object cooperatively." }
  - { constant: MISSION_CAPTURE, value: 8, input: "8", meaning: "Enter a target to capture it." }
  - { constant: MISSION_HARVEST, value: 9, input: "9", meaning: "Find and collect nearby Tiberium." }
  - { constant: MISSION_GUARD_AREA, value: 10, input: "10", meaning: "Actively guard an area." }
  - { constant: MISSION_RETURN, value: 11, input: "11", meaning: "Return to a refinery or delivery point." }
  - { constant: MISSION_STOP, value: 12, input: "12", meaning: "Stop and remain still." }
  - { constant: MISSION_AMBUSH, value: 13, input: "13", meaning: "Wait in ambush until discovered." }
  - { constant: MISSION_HUNT, value: 14, input: "14", meaning: "Search for and destroy enemies." }
  - { constant: MISSION_UNLOAD, value: 15, input: "15", meaning: "Find a location and unload cargo." }
  - { constant: MISSION_SABOTAGE, value: 16, input: "16", meaning: "Enter a target to destroy it." }
  - { constant: MISSION_CONSTRUCTION, value: 17, input: "17", meaning: "Run building buildup." }
  - { constant: MISSION_DECONSTRUCTION, value: 18, input: "18", meaning: "Run building builddown or selling." }
  - { constant: MISSION_REPAIR, value: 19, input: "19", meaning: "Repair or reload a docked vehicle, heal or promote held infantry, or send a vehicle to the repair bay." }
  - { constant: MISSION_RESCUE, value: 20, input: "20", meaning: "Clear the threats around a nominated spot, then move to the cell the house picks and guard the area there." }
  - { constant: MISSION_MISSILE, value: 21, input: "21", meaning: "Run a structure's launch sequence: a missile silo's, or an EM pulse cannon's aim and fire." }
  - { constant: MISSION_HARMLESS, value: 22, input: "22", meaning: "Remain passive and appear nonthreatening." }
  - { constant: MISSION_OPEN, value: 23, input: "23", meaning: "Open a gate and close it again once the way is clear." }
  - { constant: MISSION_PATROL, value: 24, input: "24", meaning: "Patrol assigned points." }
---

These are the mission states used by live objects. They are separate from the team-script mission opcodes listed under Mapping.

The [Do this...](/mapping/missions/tmission-do/) team mission is the only script that names one, and that page owns what becomes of the state between the script line and the team member. `QMove` is the one value that never reaches an object as itself, whichever route assigns it.

Four of the states are not available to every kind of object. `Repair` is a structure servicing what it holds or has docked. A repair bay repairs and reloads the vehicle in radio contact with it, a hospital heals the infantryman it holds, and an armory promotes one instead of healing it. A construction yard answers to the same state with a production animation and nothing more. A vehicle put on `Repair` finds the bay named by [`RepairBay`](/keys/repairbay/) and switches itself to `Enter`. `Missile` is a structure's launch sequence: a missile silo's, whichever missile the silo was given, or an EM pulse cannon's. `Open` is a gate opening and closing again, and no kind but a structure has either. `Rescue` is the one of the four that infantry, vehicles and aircraft all run. It clears the threats around a nominated spot, heads for the destination its house picks, and settles into guarding that area on arrival. A computer house calls up defenders when [its base is attacked](/systems/base-attacked/), and it puts most of them on `Rescue`, with the object under attack as the spot.

A state an object has no handling for is not refused. It becomes that object's mission and then does nothing, waking every thirty seconds to do nothing again until something else replaces it. A script that hands a team `Missile` or `Open` therefore leaves its members standing where they are.
