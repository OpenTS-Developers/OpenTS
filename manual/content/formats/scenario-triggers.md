---
format_id: scenario-triggers
title: Scenario trigger records
summary: Defines the field layout of a scenario file's trigger, event, action and tag records.
kind: file
source_files:
- code/trigtype.cpp
- code/tagtype.cpp
- code/tevent.cpp
- code/taction.cpp
- code/scenario.cpp
- code/display.cpp
- code/persist.hh
filenames:
- "<scenario>.INI"
- "*.MAP"
- "*.MPR"
related:
- type: format
  id: ini-syntax
- type: format
  id: scenario-objects
- type: system
  id: trigger-springing
---

A scenario file keeps its triggers in five sections of the map file itself. `[Triggers]` holds one row per trigger, keyed by that trigger's ID. The same ID keys the trigger's events in `[Events]` and its actions in `[Actions]`. `[Tags]` holds the tags that fire triggers, and `[CellTags]` attaches a tag to a cell. [Trigger springing](/systems/trigger-springing/) owns what the engine does with these records once they are loaded, and [scenario object records](/formats/scenario-objects/) owns which house a trigger's owner resolves to.

Every row is positional: a field's meaning comes from where it sits between the commas, never from a name. The loader reads the trigger rows and the tag rows as the scenario starts, triggers first. It registers every trigger ID before it reads any row's fields, so a row may name a trigger defined later in the file. [A worked example of all five sections](/systems/trigger-springing/#tags-triggers-and-events-in-brief) appears under trigger springing.

## The trigger row

The entry name is the trigger's ID, an arbitrary name that every other record uses to refer to the trigger. Unlike the object sections, where the loader ignores the entry name, this one is the ID. The value holds eight fields in this order:

| Position | Content |
| --- | --- |
| 1 | The owner house's name, or `<none>` for [the first house type the rules register](/formats/scenario-objects/#runtime-owners). |
| 2 | The trigger to reach from this one, by ID or display name, or `<none>` for none. [Reaching a trigger from another trigger](/systems/trigger-springing/#reaching-a-trigger-from-another-trigger) owns what the link does. |
| 3 | The trigger's display name, the name a scenario editor shows. |
| 4 | The disabled flag: `1` starts the trigger switched off, `0` switched on. The three fields after it run the other way. |
| 5 | `1` enables the trigger at easy difficulty. |
| 6 | `1` enables the trigger at medium difficulty. |
| 7 | `1` enables the trigger at hard difficulty. |
| 8 | `1` marks the trigger's tag to be handed on rather than lost with its object: to a crew member surviving the vehicle it rides on, or to whoever captures the structure it rides on. |

Only the field for the difficulty the scenario runs at is read, and [difficulty](/systems/trigger-springing/#difficulty) owns what that decides. A row whose owner resolves to no live house is deleted as it is read, and a tag naming it fires nothing.

## The event row

The entry name is again the trigger's ID. The value carries the number of events, then three fields for each event:

| Position | Content |
| --- | --- |
| 1 | How many events the row holds. The count is not checked against the fields the row actually holds. |
| 2, 5, 8, ... | The event's engine number, the same number the [event page](/mapping/events/) calls Engine ID. |
| 3, 6, 9, ... | What the next field holds: `0` a number, `1` a [team type](/mapping/team-types/). The engine writes `1` only when the event holds a team. |
| 4, 7, 10, ... | The number itself, or the team's ID or display name. |

```ini title="map file"
[Events]
01000004=1,14,0,0          ; one event on trigger 01000004
01000005=2,12,0,500,17,0,0 ; two events on trigger 01000005
```

The first row holds one event: event 14, whose parameter is the number `0`. An event whose page lists the need token none, as [mission timer expired](/mapping/events/tevent-mission-timer-expired/) does, still holds the two trailing fields, and both are `0`. A trigger with no events may leave its row out altogether; the engine's own writer then records a bare `0`.

The loader prepends each event as it parses, so the engine examines a trigger's events in the reverse of their row order, and a scenario the engine saves lists them in that reversed order. [Remembering a satisfied event](/systems/trigger-springing/#remembering-a-satisfied-event) owns what the order decides.

## The action row

The entry name is again the trigger's ID. The value carries the number of actions, then eight fields for each action:

| Position | Content |
| --- | --- |
| 1 | How many actions the row holds. |
| 2, 10, 18, ... | The action's engine number, the same number the [action page](/mapping/actions/) calls Engine ID. |
| 3, 11, 19, ... | What the next field holds: `0` a number, `1` a team, `2` a trigger, `3` a tag, or `4` a team whose time sits in the row's last field. |
| 4, 12, 20, ... | Under `0`, the number. Under `1` to `4`, the team, trigger or tag's ID, or its position in the engine's list when written in fewer than three characters, or `-1` for none. |
| 5 to 8 | Four rectangle fields: X, Y, width, height. Most actions leave all four at zero. The exceptions hold part of their parameter there and name the field on their own pages, such as [Give Credits](/mapping/actions/taction-give-credits/). |
| 9, 17, 25, ... | The waypoint the effect lands on, or the time under `4`. A missing last field leaves waypoint `A`, or time `0`. |

A trigger with no actions follows the same rule as its events: the row may be left out, and the engine's writer records a bare `0`.

## The tag row

The entry name is the tag's ID, which the loader takes as a hexadecimal number. The value holds three fields:

| Position | Content |
| --- | --- |
| 1 | The tag's lifetime: `0` volatile, `1` semi-persistent, `2` persistent. [Tag lifetimes](/systems/trigger-springing/#tag-lifetimes) owns what each kind does. |
| 2 | The tag's display name. |
| 3 | The trigger the tag fires, by ID or display name. |

A tag that fires no trigger is written as a two-field row with the display name and `<none>`, with no lifetime number at all. Reading such a row back holds the tag as volatile.

## The cell tag row

`[CellTags]` carries one row per tagged cell. The entry name is the cell, written as one number, and the value names the tag:

| Part | Content |
| --- | --- |
| Entry name | The cell number: the column plus 1000 times the row on a map whose [NewINIFormat](/keys/newiniformat/) is 4 or higher, or the column plus 128 times the row on an older map. |
| Value | The tag's ID, exactly as written in `[Tags]`. Unlike a trigger reference, this does not match a display name. A value that names no declared tag attaches a fresh empty tag, which nothing can fire, and `<none>` attaches nothing. |

Only the first row for a cell attaches: a cell already holding a tag leaves every later row for it unattached.
