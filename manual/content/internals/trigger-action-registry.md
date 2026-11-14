---
title: Trigger action registry
summary: How a trigger action is numbered, parsed, dispatched and documented, and what adding one must keep intact.
category: data-scripting
source_files:
  - code/taction.hh
  - code/taction.h
  - code/taction.cpp
  - code/need.hh
  - code/trigger.cpp
  - code/trigtype.cpp
related:
  - type: system
    id: trigger-springing
---

Each entry on a trigger's `[Actions]` line becomes one `TActionClass` record, chained to the trigger's `TriggerTypeClass` in line order. When the [trigger springs](/systems/trigger-springing/), `TriggerClass::Spring` calls each record's `TActionClass::operator()` in that order. The record's `Action` member holds a `TActionType` enumerator, and the switch in `operator()` picks the handler from it. Adding an action means adding an enumerator, a handler and the registrations below; there is no other extension layer.

## The record

An `[Actions]` line holds `count` followed by one group of eight fields per action:

| Field | Member | Meaning |
| --- | --- | --- |
| 1 | `Action` | The `TActionType` number |
| 2 | parameter code | How field 3 is read: `0` a number into `Data`, `1` a TeamType, `2` a TriggerType, `3` a TagType, `4` a TeamType with field 8 as a time |
| 3 | `Data`, `Team`, `Trigger` or `Tag` | The first parameter |
| 4 to 7 | `TriggerRect` | Resize Player View uses the rectangle. Give Credits uses `X` for the amount, Create Building At uses `X` for the building type and `Y` for the force flag, and Text Trigger uses `X` for a color position and `Y` for a display time in seconds |
| 8 | `EffectLocation` | The waypoint, or the time under parameter code `4` |

Under parameter codes `1` to `4`, field 3 names a type. `-1` names nothing, text of one or two characters is an index into that type list, and longer text is an ID. An ID that no type has yet creates a new type of that name.

Most actions that take a waypoint read it from field 8. These actions read their waypoint number from field 3 instead: Apply 100 damage at, the three Light flash actions, Reveal around waypoint, Reveal zone of waypoint, and Reduce Tiberium At.

`Read_INI` parses every action's group the same way, with no per-action code, so a new action decides only which fields its handler reads. Nothing in the game writes an `[Actions]` line back out.

The line is split on commas and empty fields are skipped, so nothing marks where one group ends and the next begins:

- Field 8 may be left out only in the last group. There the waypoint stays `A`, or the time stays `0` under parameter code `4`.
- A group other than the last that is missing any field takes its field 8 from the start of the next group, and every later group is read out of step.
- The game crashes while the scenario loads when the last group has fewer than seven fields, or when `count` is larger than the number of groups on the line.

`Data` is a union over the enumerations an action can name, so `Data.House`, `Data.Sound` and `Data.Value` are the same bits read three ways. Saves store the union as raw bytes, so a new alternative must hold no pointer and must not make the union larger.

## Numbers are serialized identities

Maps and saves store action numbers. Append new enumerators before `TACTION_COUNT`, and never reorder or reuse existing values. Numbers `0` through `105` are the stock Tiberian Sun and Firestorm set, ending at `TACTION_TALK_BUBBLE`. OpenTS adds actions `106` through `118`. `tests/actionids` pins the last stock action and each added action.

An action number with no handler does nothing when the trigger springs.

## Registration

Adding an action touches these places, all in `code/taction.hh`, `code/taction.h` and `code/taction.cpp` unless noted:

1. The enumerator, appended before `TACTION_COUNT`, with a one-line `//` comment.
2. A `{Name, Description}` row at the same position of `_ActionText`. The table is compiled only into the Debug build. The manual's scripting catalog still takes each action's name and description from it, and the manual check rejects an action whose row is missing or whose description is empty.
3. A case in `Action_Needs` naming the `NeedType` of the payload. The game does not read it; the action's parameter list and example line in the manual come from it. A payload no existing `NeedType` describes gets a new enumerator in `code/need.hh`. Catalog generation then fails until `manual/tools/scripting_engine.py` describes the new type's parameters and example line layout.
4. A case in `Attaches_To` when the action works on the objects the trigger is attached to. The game does not read an action's attach flag; the flag shows on the action's manual page. Only events decide which tag lists a trigger joins.
5. A private `TAction_<NAME>(HouseClass * house, ObjectClass * object, TriggerClass * trig, Cell const & cell)` handler returning `bool`, and an `INVOKE(<NAME>)` line in `operator()`.
6. A pin for the new number in `tests/actionids`, beside the pins for the last stock action and the other added ones.
7. In `manual/`: the permanent numeric alias in `data/scripting-route-aliases.yaml`, a change record, and an action page.

## What a handler may assume

### Arguments and result

- `house` is the trigger's owning house and is never NULL. A trigger whose owner is not in play is discarded when the scenario loads and cannot spring.
- `object` is the object whose event sprang the trigger, such as the attached object or the object that entered a cell trigger's cell. It is NULL when no object was involved, or when that object has already been removed.
- `House_From_HousesType(Data.House)` may return NULL, for a country nobody plays or a spawn position nobody holds. The handler must then skip the effect.
- The `bool` result has no effect on the game. `TriggerClass::Spring` collects it, but no caller reads the collected value, and a `false` result neither stops the later actions nor keeps the trigger armed. The existing handlers are inconsistent about it: Give Credits, Destroy all of and the one-way alliance actions return `false` for a missing house, while the mutual Make ally and Production Begins return `true`.

### Network games

Every peer in a network game runs the handler on the same frame with the same state, so a handler must change the simulation the same way on every peer. Only the local player's own result and display may depend on `PlayerPtr`. The win, lose, announce and force-end actions set the local player's outcome flags, and the special weapon actions add the sidebar button only when the trigger's house is the local player. The reveal and reshroud actions change the shroud for every house a person plays, so their effect on the simulation does not depend on `PlayerPtr`.

### Autosave requests

A handler requests an autosave with `SaveManager.Autosave.Arm()`. At the end of the frame, after removed objects are deleted, `SaveManagerClass::Service` saves a campaign or skirmish game into the next autosave slot, or submits a multiplayer save. Taking the request also restarts the timed-autosave interval. A request is not taken in these cases:

- While a multiplayer load is pending, the request waits until the load finishes.
- Once a player has left a multiplayer match, saving stays closed for the rest of the match, and the request is never written.
- During recording playback, the request is never taken.

### Object lists

`Delete_Me` takes an object off the map and marks it inactive at once, but the object stays in the object lists until the end of the frame. A handler may therefore call `Delete_Me` while walking `Technos` or `Feet`, and must skip objects that are no longer `IsActive`. `Feet` holds infantry, vehicles and aircraft, and `Technos` adds structures. Bound each loop by the count of the list it indexes.

### Alliance changes

`TAction_MAKE_ALLY_ONE_WAY` and `TAction_MAKE_ENEMY_ONE_WAY` raise `ScenarioInit` around `Make_Ally` or `Make_Enemy`, so the change is made the way the scenario loader makes it:

- `Make_Ally` skips the alliance limits and records the alliance in the house's scenario control data.
- `Make_Enemy` clears that record along with the alliance.
- Neither announces the change. `Make_Ally` also skips the rest of its in-game side effects: clearing targets that are now allies, making computer houses paranoid, and, with ally reveal on, revealing the area around the trigger's house's objects to its new ally.

The mutual `TAction_MAKE_ALLY` and `TAction_MAKE_ENEMY` leave `ScenarioInit` alone. The alliance limits apply, and outside a campaign the message list announces when a human house forms an alliance with a non-passive house or breaks one. Either way, outside a campaign `Make_Enemy` does nothing when either house is passive.
