---
title: Trigger springing
summary: "When a trigger fires on an occurrence offered to its tag, which occurrences can satisfy its events, and whether the tag survives firing."
category: maps-scenarios
keys: []
related:
  - type: system
    id: campaign-progression
  - type: system
    id: base-attacked
  - type: system
    id: ai-team-execution
  - type: system
    id: crates
  - type: event
    id: TEVENT_ANY
  - type: event
    id: TEVENT_SPIED
  - type: event
    id: TEVENT_THIEVED
  - type: event
    id: TEVENT_EVAC_CIVILIAN
  - type: action
    id: TACTION_ALLOWWIN
  - type: action
    id: TACTION_FORCE_TRIGGER
---

A trigger never checks its events by itself. It is checked only when some part of the game offers an occurrence to the tag that holds it, such as a bridge collapsing, a crate being collected or a new frame starting. Each such offer is called **springing** the tag. The trigger fires only if all of its events are satisfied during that one offer, unless earlier offers have already [marked some of them](#remembering-a-satisfied-event).

Each [event page](/mapping/events/) says what its event tests, and each [action page](/mapping/actions/) says what its action does.

## Tags, triggers and events in brief

A map describes triggers in four sections. `[Triggers]` gives each trigger's owner, its starting state and an optional linked trigger. `[Events]` and `[Actions]` list its events and actions. `[Tags]` declares the tag that holds the trigger and the tag's persistence.

A **trigger** holds the [house](/glossary/#house) that owns it, its events, its actions, whether it starts enabled, and an optional link to a second trigger. A **tag** holds one trigger, or the first trigger of a linked chain, and the persistence. Tags are what ride on objects and cells. Every offer goes to a tag, never directly to a trigger, and the tag passes it to each trigger in its chain.

```ini title="map file"
[Tags]
01000000=0,Bridge watch,02000000 ; a volatile tag holding trigger 02000000

[Triggers]
02000000=GDI,<none>,Bridge watch,0,1,1,1,0 ; owned by GDI, enabled, linked to no second trigger

[Events]
02000000=1,31,0,0 ; one event, Bridge destroyed

[Actions]
02000000=1,11,0,4,0,0,0,0, ; one action, Text Trigger... with message 4

[CellTags]
32100=01000000 ; the tag rides on cell 32100
```

A tag declared in the map exists once, however many objects and cells name it. They all share that one tag, and the tag counts how many objects and cells hold it. A tag named by a TeamType is the exception. Each team built from that type makes a separate copy of the tag and attaches it to every member it accepts, so two teams of the same type have two independent tags.

A tag can be reached through five kinds of **attachment**. Two hold the tag directly: a scenario object, and a map cell listed in `[CellTags]`. The other three are lists the tag joins when the scenario starts: the house list of the house that owns the tag's trigger, the general list, and the zone list. A tag whose events have nothing to ride on is reached through one of these lists.

The events of every trigger in a tag's chain, taken together, decide which lists the tag joins. Each [event page](/mapping/events/) states what its event may attach to, using the names `cell`, `object`, `house`, `general` and `map`, where `map` is the zone list. Because the lists are combined, one event can put a tag on a list, and that list's offers can then satisfy a different event on the same trigger. Permitting an attachment does not mean the event is ever offered there. The [springing sites below](#where-a-tag-is-offered-an-event) say where offers actually arrive.

## What springing does

When a tag is sprung, it offers the occurrence to each trigger in its chain. A trigger fires when **all of:**

- it is enabled;
- it has not been marked for destruction;
- every one of its events is satisfied during this offer.

A firing trigger runs its actions in the order `[Actions]` lists them.

Its events are examined in the reverse of their order in `[Events]`, which matters when events are [remembered](#remembering-a-satisfied-event) and when a trigger has [several countdowns](#settings-and-state-without-effect).

A tag cannot be sprung again while it is already being sprung. If a firing trigger's action causes an occurrence that the same tag watches for, that occurrence is not offered to the tag.

A disabled trigger ignores every offer but stays attached until something enables it. The disabled field in `[Triggers]` sets only the state the trigger starts in. It is separate from the three per-difficulty fields described under [Difficulty](#difficulty).

## Temporal and standing events

Every event is either standing or temporal, and the difference decides which offers can satisfy it. Each offer names the event that has just occurred, or names nothing in particular.

A **standing event** tests a condition that can be checked at any moment, such as a credit total, a global variable, the ambient light level or whether a house still has a factory. It gives the same answer whatever the offer names. Elapsed time and random delay are standing events too: they test the trigger's countdown.

A **temporal event** describes a moment, such as an object destroyed, a crate collected or a line crossed. Nothing in the world afterwards proves the moment happened, so the event accepts only an offer that names it. An offer that names anything else, or nothing, leaves the event unsatisfied even if what it describes has already happened. Apart from the [ten exempt events](#the-exempt-events), a temporal event is satisfied when **all of:**

- the offer names this event;
- any further test the event applies passes.

A standing event needs no offer written for it. Each one puts its tag on a house list or on the general list, and both lists are offered something every frame, so the event is examined every frame. The null event is the exception, because nothing satisfies it.

A temporal event is satisfied only if the part of the game that produces the occurrence makes an offer naming it. Three temporal events have no such offer and [can never be satisfied](#three-events-that-cannot-be-reached).

### The exempt events

All ten exempt events are temporal. Four of them are fully exempt: Build Building Type, Build Unit Type, Build Infantry Type and Build Aircraft Type. Each compares the type it names with the last type of that kind recorded for its house, and accepts any offer. These events put their tag on the house list, which is offered something every frame. The event is therefore satisfied from the frame the named type is recorded until the house records a different type of that kind. The event pages say what records a type.

The other six also require the offer to name them, so they behave like any other temporal event. They are cell entry, the two line crossings, zone entry, the proximity event and the attacked-by-house event.

## Where a tag is offered an event

Four parts of the game make offers, each on a different schedule:

| Site | When it runs | What it names |
| --- | --- | --- |
| The general list | Once at the start of every logic frame, before teams, objects and houses are processed | Elapsed time and random delay, every frame. Crate collection, a global or local variable change, an ambient light change and mission timer expiry, only when one has happened since the previous pass |
| A house's list | Once per house every frame, after teams and objects have been processed | Nothing in particular. [Damage to a base](/systems/base-attacked/) also offers the attacked event to this list when it happens |
| Cells, and the zone list | Each time an uncloaked infantry or vehicle finishes moving into a cell | Cell entry, to that cell's tag. The two line crossings, to every cell along the row or column whose tag watches for that crossing, when the cell entered is on a crossing line. Zone entry, to each zone tag whose cell shares a [movement zone](/glossary/#movement-zone) with the destination of the infantry or vehicle |
| An object's tag | At the moment the occurrence happens to that object | The one event that has just happened |

Team processing also offers the team-left-map event to every tag on the general list when it disposes of an empty team that was leaving the map. [Leaves map (team)...](/mapping/events/tevent-leaves-map/) says when that happens.

A tag on the general list that fires on one of these offers gets no further offers from that frame's pass over the list. When firing destroys that tag, the tag listed after it also misses that frame's pass.

The extra offers for variables, ambient light and the mission timer change nothing for the standing events they name, because every tag on the list is already examined on the elapsed-time offer each frame. Crate collection is the one temporal event these offers name. A crate picked up during a frame is offered to the general list at the start of the next frame.

Flying aircraft never make cell or zone offers. Infantry or a vehicle passing under a bridge does not trigger cell entry for the cells beneath it. Infantry or a vehicle on the bridge does.

A cell where a horizontal and a vertical line cross always offers the horizontal crossing. It offers the vertical crossing only if a vertical line also passes through the last cell of that row.

Only tags whose triggers include zone entry receive offers from the zone list, so in practice the zone list is the list of zone tags.

Offers to an object's tag are the most numerous and the least uniform. Each is made where the game handles the occurrence, such as damage, destruction or discovery, and names exactly one event. A temporal event can therefore be satisfied only where such an offer exists. An event that permits an object attachment is not necessarily offered through one.

## Remembering a satisfied event

A trigger can collect its temporal events over several offers. When a temporal event is satisfied during a remembering offer, it is **marked**. A marked event counts as satisfied on every later offer without being tested again. Marking is how one trigger combines occurrences that never arrive in the same offer.

A satisfied event is marked when **all of:**

- the offer is a remembering offer;
- the event is temporal;
- the event can be remembered.

An offer is a remembering offer when the tag is persistent. An offer to a tag of any persistence also becomes remembering partway through when certain events are satisfied during it. They are cell entry, either line crossing, zone entry, the team-left-map event, the building-exists event and the four build events. Remembering then stays on for the rest of that trigger's examination. Events are examined in the reverse of their order in `[Events]`, so such an event can mark only the events written before it on that line, and itself when it can be remembered.

Five events can never be remembered and are tested again on every offer: the two attacked events, cell entry, the paralyzed event and the repeating spotlight event. The plain spotlight event can be remembered, which is the only difference between the two spotlight events.

Without marking, every event must be satisfied during a single offer. Two temporal events, such as an object destroyed and a crate collected, can then never be satisfied together, because each offer names only one event. Give such a trigger a persistent tag. Each temporal event is then marked when it happens and stays marked, and the trigger fires on the offer that satisfies the last event outstanding.

A mark is never cleared. Elapsed time and random delay are standing events and are never marked. Their countdown restarts:

- when the trigger is created;
- whenever Enable Trigger names it, even if it is already enabled;
- when a global or local variable that one of its events names changes value;
- on every remembering offer that satisfies all of its events.

The last case is what makes a persistent timed trigger fire repeatedly.

## Tag lifetimes

A tag's persistence decides what happens after its triggers fire. It is the number before the tag's name in `[Tags]`: `0` volatile, `1` semi-persistent, `2` persistent.

| Persistence | On firing | Afterwards |
| --- | --- | --- |
| Volatile | Fires on the first offer that satisfies the trigger | The tag is destroyed with all of its triggers and removed from everything it rode on |
| Semi-persistent | Fires only on an offer made while the tag has one attachment left. An earlier offer that satisfies the trigger fires nothing | An earlier satisfying offer made for an object or cell that holds the tag removes the tag from that object or cell. The firing offer destroys the tag |
| Persistent | Fires on every offer that satisfies the trigger | Nothing changes. The tag stays where it is and can fire again |

Semi-persistent fires on the last of a group instead of the first. A team's tag, for example, goes onto every member the team accepts. A member drops out of the count when it satisfies the trigger. A member of a computer player's team also drops out when it leaves the team, which includes being destroyed. The trigger fires for the last member still counted. A team can restrict its tag to members that can carry passengers. Only those members then receive the tag and count toward it.

Any other object holding the tag that leaves the game without satisfying the trigger still counts. The count then never falls to one while a living holder remains, so the tag never fires.

Three offers are made for the moving unit, not for the holder of the tag:

- Infantry entering a building or vehicle never lowers the count of that building's or vehicle's tag.
- A line crossing lowers the count only when the cell the unit entered holds the tag.
- Zone entry lowers the count only when the unit's destination holds the tag.

:::caution[Semi-persistent fires nothing unless the tag rides on an object or a cell]
The count a semi-persistent tag waits on includes only objects and map cells. Joining a house list, the general list or the zone list adds nothing to it. A semi-persistent tag that rides on no object or cell stays at zero, never reaches one, and never fires for the whole scenario. Events with nothing to ride on, such as elapsed time, credit totals and global variables, need a volatile or a persistent tag.
:::

A destroyed tag and its triggers stop taking offers at once but are released only at the end of the frame. The triggers that fired are released first, and the tag after them. As each trigger is released, the tag's link moves on to the next trigger in its chain. A tag that held a single trigger therefore has no trigger left when its release runs, which decides the Allow Win case below.

## Holding back the victory

In a campaign or standalone mission, [Allow Win](/mapping/actions/taction-allowwin/) holds back its house's victory. The action does nothing when it runs. The hold is counted before the first frame, and it is lifted only when the tag is released while it still holds a trigger. The Allow Win page owns the count and [the release that does lift it](/mapping/actions/taction-allowwin/#destroying-the-tag-instead).

:::caution[Firing a single-trigger tag does not lift the hold]
A tag with a single trigger, the usual shape, loses its link to that trigger before its release runs. Firing therefore destroys a volatile or semi-persistent tag of that shape and leaves the hold in place. Firing lifts the hold only when at least one trigger in the tag's chain did not fire and is still linked when the tag is released.
:::

A tag that is never released holds the victory the same way. That includes:

- a persistent tag, which firing never destroys;
- a volatile or semi-persistent tag whose trigger is never satisfied or stays disabled;
- a tag declared in the map that rides on nothing that receives offers;
- a semi-persistent tag that rides on no object or cell, which never fires or dies even when its trigger is satisfied.

Unless [Destroy Tag](/mapping/actions/taction-destroy-tag/) removes such a tag, the house the hold is charged to cannot win the mission for the rest of the game.

## Reaching a trigger from another trigger

Five actions work on triggers and tags by name, not on whatever sprang them. [Enable Trigger](/mapping/actions/taction-enable-trigger/) and [Disable Trigger](/mapping/actions/taction-disable-trigger/) switch every trigger of the named type on or off. [Destroy Trigger](/mapping/actions/taction-destroy-trigger/) removes every trigger of the named type permanently. [Destroy Tag](/mapping/actions/taction-destroy-tag/) removes every tag of the named type, so whatever those tags rode on is no longer linked to them.

[Force Trigger](/mapping/actions/taction-force-trigger/) fires every trigger of the named type without examining its events, so it fires even a trigger whose events can never be satisfied. It has two limits:

- The trigger fires without an object or a cell, so actions that work on the thing the trigger is attached to have nothing to work on.
- The tag is bypassed: its persistence is ignored, nothing is detached and no tag is destroyed. A forced volatile trigger therefore stays in place and can be forced again.

Force Trigger does not fire a disabled trigger or one already marked for destruction.

## Three events that cannot be reached

:::caution[Spied upon, Thieved by... and Civilians Evacuated can never be satisfied]
[Spied upon](/mapping/events/tevent-spied/), [Thieved by...](/mapping/events/tevent-thieved/) and [Civilians Evacuated](/mapping/events/tevent-evac-civilian/) are temporal and not exempt, so each needs an offer that names it. No part of the game makes one. Every offer that reaches such a trigger names another event or nothing in particular.

A trigger with any of these events never fires unless forced, even when its other events work, because every event on a trigger must be satisfied.
:::

The house conditions that two of these events test are still recorded, which makes the events look usable. Losing a structure to an engineer [marks the losing house as robbed](/systems/capture/#capturing-a-non-allied-structure). An aircraft that retreats out of the playable area with [a civilian](/keys/civilian/) aboard marks that passenger's house as having evacuated a civilian. Neither mark is ever cleared, and nothing reads either one except these unreachable events.

[Force Trigger](/mapping/actions/taction-force-trigger/) is the only way to run the actions of a trigger that uses one of the three.

## Difficulty

`[Triggers]` holds three per-difficulty fields for each trigger: easy, normal and hard, in that order. Only the field for the difficulty being played is read. If it is `0`, the trigger starts disabled and stays disabled: [Enable Trigger](/mapping/actions/taction-enable-trigger/) does not enable it, and Force Trigger does not fire it.

A campaign mission uses the difficulty the player chose. A skirmish or multiplayer game uses the computer skill set in the lobby.

A saved game restores trigger state as it was saved, including which triggers are enabled, which events are marked, and how many attachments each semi-persistent tag still has.

## Settings and state without effect

A trigger has one countdown, shared by all of its events. When a trigger has more than one elapsed time or random delay event, each restart sets the countdown from every one of them in turn. The event written first in `[Events]` is applied last, so its delay is the one that takes effect, and the other delays are ignored.
