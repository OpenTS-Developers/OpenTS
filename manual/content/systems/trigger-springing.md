---
title: Trigger springing
summary: "Fires a trigger when every event on it is satisfied in one pass, runs its actions, and then keeps or destroys the tag that held it."
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

Triggers are not examined on a schedule of their own. Something that happens in the game, such as a bridge collapsing, a countdown reaching zero or a house passing a credit total, is offered to the tag that holds the trigger by whatever part of the game produced it. The trigger fires only if everything it waits for is satisfied during that one offer. That offer is called **Springing**: handing a tag something that has just happened, so that the triggers riding on the tag can decide whether to fire.

What follows is the machinery around that offer: how an event is satisfied, which parts of the game make the offers, what a satisfied event is remembered as, and what becomes of the tag afterwards. What each event tests belongs to [the event's own page](/mapping/events/), and what each action does to [the action's own](/mapping/actions/).

## Tags, triggers and events in brief

A trigger's owner and starting state are written in `[Triggers]`, with its events in `[Events]` and its actions in `[Actions]`. Its persistence is written in `[Tags]`. The engine keeps those as separate records, and which record holds what decides most of the behavior below.

A **trigger** holds the [house](/glossary/#house) that owns it, its events, its actions, whether it starts enabled, and an optional link to a second trigger. A **tag** names one trigger, or the head of a chain of linked triggers, and holds the persistence. The tag is the part that rides on something. Nothing offers an event to a trigger directly: everything is offered to a tag, which passes it along the chain.

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

A tag declared in the map exists once, however many things name it. Every object and cell that names the same tag shares one record, and the record counts its holders. The one exception is a tag named by a TeamType. Each team built from that type makes a private tag of its own and attaches it to every member it accepts, so two teams of one type have two independent tags.

An **attachment** is a thing the tag rides on, and there are five kinds. The first two hold the tag directly: a scenario object, and a map cell listed in `[CellTags]`. The other three are lists the tag joins as the scenario starts: one per house, one general list, and one list of zone tags. A tag whose events have nothing to ride on lives on one of those.

Which of the five a tag joins is derived from every event and every action of every trigger in its chain, combined. A tag therefore reaches a list because of one event, and is then offered occurrences that a different event on the same trigger can answer. An event can also permit more than one kind of attachment, and permitting one is not the same as being reachable there. The attachment set decides which lists the tag joins: each [event page](/mapping/events/) states what its event may attach to. Those pages name the attachments `cell`, `object`, `house`, `general` and `map`, and `map` is the list of zone tags. The [springing sites below](#where-a-tag-is-offered-an-event) say where an offer will actually arrive.

## What springing does

Springing a tag walks its chain of triggers and offers each one the occurrence. A trigger takes the offer under **all of:**

- it is enabled;
- it has not been marked for destruction;
- **all of** its events are satisfied by that one offer.

Every event has to be satisfied; there is no way to write an alternative. Only when the last of them agrees does the trigger fire. Firing runs every action on it, in the order the map file lists them. The events are examined in the reverse of that order, which matters in two places, described [under memory](#remembering-a-satisfied-event) and again [under settings](#settings-and-state-without-effect).

A tag will not spring while it is already springing. An action that causes something a trigger of the same tag watches for therefore cannot set that tag off again from inside its own firing.

Disabling a trigger stops it taking any offer at all; it stays attached to whatever it was on and does nothing until something enables it again. The map file's disabled flag decides only the state a trigger starts in; it is a different field from the three per-difficulty ones under [Difficulty](#difficulty).

## Temporal and standing events

Every event is one of two kinds, and the difference decides whether the event can be reached at all.

A **standing event** describes a condition the engine can examine at any moment: a credit total, a global variable, the ambient light level, whether a house still holds a factory. Whatever offer arrives, a standing event simply looks at the thing it watches and answers. It does not care what the offer was about.

A **temporal event** describes a moment: an object destroyed, a crate collected, a line crossed. Nothing about the world afterwards proves it happened, so the engine only accepts it when the offer names that exact event. An offer about something else, or an offer that names nothing in particular, leaves a temporal event unsatisfied even where the thing it describes has already occurred.

That gate is the whole of the distinction. Ten events are [exempt from it](#the-exempt-events); every other temporal event is satisfied under **all of:**

- the offer names this same event;
- whatever further test the event applies is passed.

Two consequences run through everything else on this page. Set the null event aside, since nothing ever satisfies it. Every standing event places its tag on a house list or on the general list, and both of those are offered something every frame, so a standing event needs nothing written for it and is examined constantly. A temporal event needs one particular offer, written into the part of the game that produces the occurrence. An event with no such offer written for it can never be satisfied, however faithfully its own test would answer. Three of them are in that position, and they are [covered below](#three-events-that-cannot-be-reached).

### The exempt events

All ten are temporal events. Four of them gain the whole exemption. Build Building Type, Build Unit Type, Build Infantry Type and Build Aircraft Type each read the record of what their house most recently completed and compare it against the type the event names. Any offer that reaches the tag will do. Those four attach to a house, the house list is offered something every frame, and the comparison catches the frame on which the house finished the named type.

The other six impose the naming requirement again in their own tests, so they behave like any other temporal event. They are the cell entry event, the two line crossing events, the zone entry event, the proximity event and the attacked-by-house event.

## Where a tag is offered an event

Four parts of the game make offers, on four different cadences. The table gives when each runs and what it names.

| Site | When it runs | What it names |
| --- | --- | --- |
| The general list | Once at the top of every logic frame, before teams, objects and houses take their turns | Elapsed time and random delay every frame; crate collection, global and local variable changes, ambient light changes and mission timer expiry only on the frame that produced each |
| A house's list | Once per house, at the end of the logic frame, after objects have taken their turns | Nothing in particular. [Damage to a base](/systems/base-attacked/) names the attacked event on the same list separately |
| Cells, and the zone list | Each time an uncloaked infantry, vehicle or aircraft reaches the center of a new cell | Cell entry on that cell's own tag; the two line crossings, on every tagged cell along the row or the column, when the cell reached is marked as a crossing line; and zone entry on each zone tag whose cell shares a [movement zone](/glossary/#movement-zone) with the destination |
| An object's own tag | At the moment the occurrence happens to that object | The one event that has just happened |

The general list is offered elapsed time and random delay with no condition attached, so a tag on that list is offered something every frame however few of its events are time based. A tag whose trigger fires on one of the offers made during that walk is not given the later offers in the same frame.

The conditional offers on that list are gated by a mark set when the occurrence happens, and those marks are cleared once the walk over the list has finished. A variable changed by a trigger part-way through the walk is therefore offered only to the tags the walk has not yet reached. The tags it has already passed do not receive that change on the following frame either, because the mark recording it is gone by then. A change made anywhere outside that walk is waiting when the next one begins, and every tag on the list sees it.

The zone list is offered only to tags whose triggers watch for zone entry, so it is in practice the list of zone tags rather than a general map list.

Offers made to an object's tag are the largest group and the least uniform. Each is written at the point in the game that produces the occurrence, such as the damage handling, the destruction handling or the discovery handling, and each names exactly one event. This is why a temporal event is reachable only where somebody wrote the offer for it, and why an event that permits an attachment is not thereby reachable through it.

## Remembering a satisfied event

A temporal event can be marked off, so that a later offer finds it already satisfied and does not re-examine it. Marking is what lets one trigger combine occurrences that can never arrive in one offer.

An event is marked off under **all of:**

- the offer is a remembering one;
- the event is temporal;
- the event admits being remembered.

An offer is a remembering one when the tag is persistent. It also becomes one part-way through, when certain events agree: cell entry, the two line crossings, zone entry, the team-left-map event, the building-exists event and the four build events. Any of those switches remembering on for the rest of that offer. The events are examined in the reverse of their order on the trigger's line in `[Events]`. An event that switches remembering on this way therefore reaches only the events written *before* it on that line.

Five events refuse to be remembered and are re-examined on every offer: the two attacked events, cell entry, the paralyzed event, and the repeating spotlight event. The plain spotlight event is remembered normally, and that is the whole difference between the two spotlight events.

**A volatile or semi-persistent trigger must have all of its events true during a single offer.** Pairing two temporal events, say an object destroyed and a crate collected, asks for both occurrences in one offer. One offer names one event. **A persistent trigger accumulates them instead.** Each temporal event is marked off as it happens and stays marked, so the trigger fires on the offer that satisfies the last one outstanding.

Marking is never undone except for elapsed time and random delay, which are rearmed whenever the trigger's timer restarts. The timer restarts when the trigger is created, when it is enabled, when a global or local variable the trigger watches changes value, and on any offer that satisfies every event of a remembering trigger. That last one is what makes a persistent timed trigger repeat.

## Tag lifetimes

The tag's persistence decides what happens after its triggers fire. Every tag is one of three kinds, declared in `[Tags]` as the number before the tag's name: `0` volatile, `1` semi-persistent, `2` persistent.

| Persistence | On firing | Afterwards |
| --- | --- | --- |
| Volatile | Fires on the first offer that satisfies the trigger | The tag comes off the object or cell that sprang it, where there was one, and is destroyed, taking its triggers with it |
| Semi-persistent | Fires only when one attachment is left; an earlier offer that satisfies the trigger fires nothing | The offers before the last one detach the tag from whatever sprang them; the last one destroys it |
| Persistent | Fires on every offer that satisfies the trigger | Nothing. The tag stays where it is and fires again |

Semi-persistent is the lifetime for going off on the last of a group rather than the first. A team's tag is attached to every member the team accepts, drops one member each time that member satisfies the trigger, and fires on the one attachment left. A team can also restrict the tag to members that can carry passengers, in which case those are the only members it goes onto and the only ones counted.

:::caution[Semi-persistent fires nothing unless the tag rides on an object or a cell]
The count a semi-persistent tag waits on counts objects and map cells only. Placement on a house list, on the general list or on the zone list adds nothing to it. A semi-persistent tag that rides on neither an object nor a cell therefore sits at a count of zero, and never reaches the count of one it fires at. Every offer it takes leaves it exactly as it was, for the whole scenario. Elapsed time, credit totals, global variables and every other event with nowhere to ride need a volatile or a persistent tag.
:::

A destroyed tag and its triggers are not disposed of on the spot. They stop taking offers immediately and are released at the point in the frame where nothing is still walking a list of them. The triggers that fired go first and the tag follows. Each trigger released this way unhooks itself from the tag on the way out, moving the tag's link on to the next trigger in its chain. A tag that held a single trigger therefore reaches its own release naming nothing at all. That ordering is what the next section turns on.

## Holding back the victory

[Allow Win](/mapping/actions/taction-allowwin/) is the one action whose outcome a tag's disposal settles, so the three lifetimes above decide whether the hold it places is ever lifted. The action does nothing at all when it runs. The hold it places on its house's victory is counted before the first frame, and the only thing that lifts it is the tag being disposed of while it still names a trigger. That page owns the count and the disposal that does clear it.

:::danger[Dying on firing does not lift the hold]
The release order above decides this. A tag holding a single trigger, which is the ordinary shape, has lost its link to that trigger before its own release runs. Firing therefore disposes of the tag and leaves the hold standing. Only a tag holding a chain that keeps a trigger through its own release lifts the hold by firing.
:::

Everything that keeps such a tag from being disposed of at all holds the victory the same way, and there are several. A persistent tag never dies. Neither does a volatile or semi-persistent tag whose trigger is never satisfied, one that is left disabled, or one declared in the map and attached to nothing that ever receives an offer. The same is true of a semi-persistent tag riding on neither an object nor a cell. It takes offers, its trigger is satisfied, and it still neither fires nor dies, because the count it waits on never reaches one. Each of these leaves the house the hold is charged to unable to win the mission for the rest of the game.

## Reaching a trigger from another trigger

Five actions operate on triggers and tags by name rather than on whatever sprang them. [Enable Trigger](/mapping/actions/taction-enable-trigger/) and [Disable Trigger](/mapping/actions/taction-disable-trigger/) switch every trigger of the named type on and off. [Destroy Trigger](/mapping/actions/taction-destroy-trigger/) removes them permanently; nothing brings a destroyed trigger back. [Destroy Tag](/mapping/actions/taction-destroy-tag/) removes every tag of the named type, so whatever those tags rode on loses its link to them.

[Force Trigger](/mapping/actions/taction-force-trigger/) fires every trigger of the named type outright. It is the one route that ignores events completely: the trigger's own events are not examined, so a trigger whose events could never be satisfied still fires. Two limits come with it. The forced trigger is fired without an object or a cell, so any of its actions that work on the thing a trigger is attached to have nothing to work on. And the tag is bypassed entirely: persistence is not read, nothing is detached, and no tag is destroyed, so forcing a volatile trigger leaves it in place to be forced again.

A disabled trigger cannot be forced.

## Three events that cannot be reached

:::danger[Spied upon, Thieved by... and Civilians Evacuated can never be satisfied]
[Spied upon](/mapping/events/tevent-spied/), [Thieved by...](/mapping/events/tevent-thieved/) and [Civilians Evacuated](/mapping/events/tevent-evac-civilian/) are all temporal and none of them is exempt from the gate. Each therefore needs an offer naming it, and no part of the game makes one. Every offer that reaches such a trigger names something else or names nothing in particular, and the gate rejects it before the event's own test is read. A trigger whose events include any of the three never fires by itself. Neither does one that pairs one of them with events that do work, because every event on a trigger has to be satisfied.

Both house marks behind those events are maintained, which is what makes the events look alive. Losing a structure to an engineer [marks the losing house as robbed](/systems/capture/#capturing-a-non-allied-structure), and an aircraft that retreats outside the playable area with [a civilian](/keys/civilian/) aboard marks that passenger's house. Neither mark is ever cleared, and nothing reads either one except the event that cannot be reached. The spy event has no test of its own beyond the gate: were an offer ever made for it, it would be satisfied on the spot.

Force Trigger is the only way to get any consequence out of a trigger built on one of the three, and it fires the actions regardless of the event rather than because of it.
:::

## Difficulty

A trigger record holds three per-difficulty fields in `[Triggers]`, in easy, normal and hard order, and only the one for the difficulty the scenario is being played at is read. A trigger whose field reads `0` there is disabled as it is created, so it never springs, and [Enable Trigger](/mapping/actions/taction-enable-trigger/) leaves it alone rather than bringing it back.

A campaign mission is played at the difficulty the player chose. A skirmish or multiplayer game is played at the one the lobby's computer skill sets. A saved game keeps the state it was stored with, including which triggers are enabled, which events are marked off, and how many attachments a semi-persistent tag still has.

## Settings and state without effect

A trigger keeps one countdown, not one per event. A trigger with more than one elapsed time or random delay event has each of them overwrite that single countdown as the list is walked. The delay that takes effect is therefore the one written first in the map file, and the others are inert.

Springing sets a flag for firing a tag's triggers without examining their events, and nothing sets it. Force Trigger, the action that would want it, reaches the trigger directly instead.
