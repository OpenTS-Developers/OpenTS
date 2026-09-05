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

A trigger action is one `TActionClass` record hanging off a `TriggerTypeClass`, created for each entry of a trigger's `[Actions]` line and carried out by `TActionClass::operator()` when the trigger springs. The action's number is the `TActionType` enumerator, and the record is the extension point: a new action is a new enumerator, a handler, and the registrations below. There is no separate plug-in layer.

## The record

An `[Actions]` line holds `count` followed by one group of eight fields per action:

| Field | Member | Meaning |
| --- | --- | --- |
| 1 | `Action` | The `TActionType` number |
| 2 | parameter code | How field 3 is read: `0` a number into `Data`, `1` a TeamType, `2` a TriggerType, `3` a TagType, `4` a TeamType with field 8 as a time |
| 3 | `Data`, `Team`, `Trigger` or `Tag` | The first parameter |
| 4 to 7 | `TriggerRect` | Resize Player View uses the rectangle; Give Credits uses `X` for the amount, and Create Building At uses `X` for the building type and `Y` for the force flag |
| 8 | `EffectLocation` | The waypoint, or the time under parameter code `4`. A missing field leaves waypoint `A` |

`Data` is a union over the enumerations an action can name, so `Data.House`, `Data.Sound` and `Data.Value` are the same bits read three ways. `Read_INI` and `Build_INI_Entry` are symmetric and know nothing about individual actions; a new action decides only how its handler reads the fields.

## Numbers are serialized identities

Action numbers are stored in maps and saves. Append new enumerators before `TACTION_COUNT`; never reorder or reuse existing values. Numbers `0` through `105` are the stock Tiberian Sun and Firestorm set, ending at Talk Bubble. OpenTS adds actions `106` through `118`. `tests/actionids` pins the last stock action and each added action.

An unknown action number does nothing when dispatched.

## Registration

Adding an action touches these places, all in `code/taction.hh`, `code/taction.h` and `code/taction.cpp` unless noted:

1. The enumerator, appended before `TACTION_COUNT`, with a one-line `//` comment.
2. A `{Name, Description}` row at the same position of `_ActionText`. The table is compiled only into the Debug build, but the manual's scripting catalog is generated from its text, so the row is mandatory and its description must not be empty.
3. A case in `Action_Needs` naming the `NeedType` of the payload. A payload no existing `NeedType` describes gets a new enumerator in `code/need.hh`; the manual generator then refuses to run until `manual/tools/scripting_engine.py` describes its parameters and the example line layout.
4. A case in `Attaches_To` when the action works on the objects the trigger is attached to. The flag only widens where a tag may be placed, so it never removes an action from a house or general list.
5. A private `TAction_<NAME>` handler with the common signature, and an `INVOKE(<NAME>)` line in `operator()`.
6. In `manual/`: the permanent numeric alias in `data/scripting-route-aliases.yaml`, a change record, and an action page.

## What a handler may assume

- `house` is the trigger's owning house and is never NULL; a trigger whose owner resolved to nothing does not spring. `House_From_HousesType(Data.House)` may still return NULL, for a country nobody plays or a spawn position nobody holds, and a handler returns `false` in that case rather than acting.
- Every peer in a network game runs the handler on the same frame with the same state, so a handler mutates shared simulation state only and never branches on `PlayerPtr` except to affect presentation. A trigger arms an autosave with `SaveManager.Autosave.Arm()`; `SaveManagerClass::Service` writes it after deferred deletion, using the timed-save slots and multiplayer save boundary.
- Objects are retired at the end of the frame, so a handler may call `Delete_Me` or destroy objects while walking the object lists. `Feet` holds infantry, vehicles and aircraft and `Technos` adds structures; a loop is bounded by the count of the list it indexes.
- Raising `ScenarioInit` around `Make_Ally` or `Make_Enemy` makes the alliance as the scenario loader would: the ally action skips alliance limits, both actions update the scenario control record when an alliance changes, and neither announces the change. The enemy action still excludes passive houses outside a campaign.
