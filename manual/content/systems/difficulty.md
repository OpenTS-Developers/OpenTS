---
title: Difficulty settings and handicaps
summary: "Turns the chosen difficulty into one slot per house and scales that house's damage, speed, armor, rate of fire, prices and build times by the figures the slot holds."
category: ai-teams
keys:
  - AIHateDelays
  - Armor
  - BuildTime
  - CompEasyBonus
  - Cost
  - DestroyWalls
  - Difficulty
  - FillEarliestTeamProbability
  - FirePower
  - Firepower
  - GameSpeedBias
  - Groundspeed
  - MaximumAIDefensiveTeams
  - MinimumAIDefensiveTeams
  - MultiplayerAICM
  - PlayerControl
  - ROF
  - RepairDelay
  - TeamDelays
  - TotalAITeamCap
related:
  - type: system
    id: production
  - type: system
    id: repair
  - type: system
    id: ai-team-production
  - type: system
    id: ai-base-building
  - type: system
    id: target-selection
  - type: system
    id: trigger-springing
---

## From the setting to a slot

This section introduces the difficulty slot the rest of the page turns on. Anyone who already knows that a computer house is handed the inverse of the setting the player chose can skip to [what one difficulty section sets](#what-one-difficulty-section-sets).

A scenario carries two difficulty slots. In a campaign game both come from [`Difficulty=`](/keys/difficulty/), the campaign setting kept in `sun.ini`; outside a campaign both come from the difficulty chosen for the session. One slot is the setting itself and the other is `2` minus it, so the two always move in opposite directions.

A slot selects one of three sections in the rules: `[Easy]` is slot 0, `[Normal]` slot 1 and `[Difficult]` slot 2. The section names describe the house reading them rather than the player's skill.

The table gives the section each kind of house ends up reading, for each setting a player can choose. Read the two right-hand columns against each other: they are mirror images, and every later table on this page is a consequence of that one step.

| Setting chosen | A player-controlled house in a campaign | Every other house |
| --- | --- | --- |
| Easy | `[Easy]` | `[Difficult]` |
| Normal | `[Normal]` | `[Normal]` |
| Hard | `[Difficult]` | `[Easy]` |

Outside a campaign game the right-hand column reaches computer houses only. Every human house is handed slot 1 whatever the session was set to, so the setting tunes the computer alone there.

A campaign mission names its difficulty in a message as it starts. The name says how hard the mission is, which is the mirror of the section the computer reads: a mission whose computer houses read `[Easy]` is announced as Hard, `[Normal]` as Medium and `[Difficult]` as Easy. A [launch file](/formats/spawn-ini/#what-a-player-is-shown) may give the setting a name of its own instead, which is how a client offering more difficulties than the game's three names the one it chose.

In a campaign the handicap is given to each house as the scenario's `[Houses]` list is read, one house at a time, and the only thing that puts a house in the left-hand column is [`PlayerControl=yes`](/keys/playercontrol/) in that house's own section. More than one house may carry it, and each one that does takes the player's column.

:::caution[A campaign map that omits `PlayerControl` leaves the player wearing the computer handicap]
The house named by the map's `[Basic] Player=` entry is not resolved until after every house has been created and handicapped, and the player-control flag that entry sets arrives too late for the test. A campaign map that names a player house without also setting `PlayerControl=yes` in that house's own section therefore runs the whole mission with the player's own house in the right-hand column — on `[Difficult]` at the Easy setting. Nothing re-assigns it during that mission; the next scenario reads and handicaps its own houses again.
:::

## What one difficulty section sets

A difficulty section carries eight settings that reach a decision, and the table gives all eight together with what a house in that slot does with each. Each of the six multipliers states in its own row which way a figure above 1 moves, because the direction is not the same for all of them: above 1 makes a house deal more damage, move faster and survive longer, but it also makes that house fire more slowly, pay more and build more slowly. None of the figures is inverted the way the slot is — a house simply reads what its own section holds.

| Setting | Effect on a house in that slot |
| --- | --- |
| [`FirePower=`](/keys/firepower-difficulty-settings/) | Multiplies the damage its objects deal by firing a weapon; above 1 deals more. Projectiles the engine creates outside that path — a nuke silo, either EM pulse, a superweapon, a trigger action, a splitting bullet — are never scaled |
| [`Groundspeed=`](/keys/groundspeed/#scope-difficulty-settings) | Multiplies the speed of its ground movement; above 1 travels faster |
| [`Armor=`](/keys/armor/#scope-difficulty-settings) | Divides the damage its objects take; above 1 survives longer |
| [`ROF=`](/keys/rof/#scope-difficulty-settings) | Multiplies the delay between its shots; above 1 fires more slowly |
| [`Cost=`](/keys/cost/#scope-difficulty-settings) | Multiplies every price it pays; above 1 pays more |
| [`BuildTime=`](/keys/buildtime/#scope-difficulty-settings) | Multiplies the build time of everything it produces; above 1 builds more slowly |
| [`RepairDelay=`](/keys/repairdelay/) | Scales the wait a computer house takes between starting one building repair and the next, the result being drawn at random between a quarter of the scaled figure and twice it, in minutes |
| [`DestroyWalls=`](/keys/destroywalls/) | `no` stops its computer objects [scoring walls as targets](/systems/target-selection/#picking-the-winner) |

```ini title="rules.ini"
[Difficult]
FirePower=1.2
Groundspeed=1.1
Armor=1.2
ROF=0.9
Cost=0.9
BuildTime=0.9
RepairDelay=.02
DestroyWalls=yes
```

The first seven are folded into figures the house keeps. `DestroyWalls` is read from the section itself at the moment the decision is made, so no country setting and no per-house value can shift it.

:::caution[A section absent from every file leaves its figures at zero]
The three sections start at zero — every multiplier `0`, the delays `0`, the flags `no` — and each is read only out of a file that actually carries it. A rules tree in which `[Difficult]` never appears therefore leaves a house in that slot dealing zero damage, moving at zero speed and paying zero for everything. Incoming damage is divided by the armor figure, so every hit that house takes divides by zero and then lands on the engine's floor of 1 point, which makes the house nearly invulnerable rather than crashing. `DestroyWalls` comes out `no` rather than `yes` at the same time. Nothing supplies these figures except a file carrying the section.
:::

:::caution[A later file that carries the section resets the rest of the block]
Each section is re-read from fixed built-in values, never from the values already in force, every time a file carrying that section is processed — [each rules file in turn](/formats/rules-registries/), then the map, and in a campaign the scenario's own companion file after that. A map that declares `[Difficult]` to change one multiplier restores every other setting in that section to its built-in default and discards what the rules files put there. Only repeating the whole block keeps it.
:::

## How the figures are combined

The seven copied figures are worked out once, at the moment the house is given its slot, and not per shot, per order or per frame. The table gives what each of the seven is combined with in each mode; the difference to read off it is that outside a campaign game the house's country section contributes a second multiplier to six of them, and a campaign game drops that contribution and keeps everything else.

| Difficulty setting | Campaign game | Outside a campaign |
| --- | --- | --- |
| `FirePower=` | On its own | Times the country's [`Firepower=`](/keys/firepower-housetype/) |
| `Groundspeed=` | Times [`GameSpeedBias`](/keys/gamespeedbias/) | Times the country's [`Groundspeed=`](/keys/groundspeed/#scope-housetype) and `GameSpeedBias` |
| `Armor=` | On its own | Times the country's [`Armor=`](/keys/armor/#scope-housetype) |
| `ROF=` | On its own | Times the country's [`ROF=`](/keys/rof/#scope-housetype) |
| `Cost=` | On its own | Times the country's [`Cost=`](/keys/cost/#scope-housetype) |
| `BuildTime=` | Times `GameSpeedBias` | Times the country's [`BuildTime=`](/keys/buildtime/#scope-housetype) and `GameSpeedBias` |
| `RepairDelay=` | Taken as written | Taken as written |

The country's own multipliers sit in its HouseType section and each default to 1, so a rules tree that never sets them leaves the two columns identical. [How long it takes](/systems/production/#how-long-it-takes) places the build-time figure in the production chain, and [when the computer repairs](/systems/repair/#when-the-computer-repairs) covers what the repair delay does with its value.

## When a house is re-handicapped

- **Campaign scenario load or restart.** Each house in the scenario's `[Houses]` list, as it is created. Applying carry-over state afterward does not replace that handicap.
- **Every other mode.** As the session's houses are assigned: humans to slot 1, computer houses to the inverted slot minus the bonus below. The `Neutral` and `Special` houses created alongside them are never handicapped and keep a multiplier of 1 throughout.
- **A house passing to the computer.** The house is re-handicapped with `2` minus the slot it already holds.

That last one runs only on a house a person was playing, and it is the same inversion applied a second time. The table traces a campaign house through it. The right-hand column is exactly what [the table above](#from-the-setting-to-a-slot) gives every other house at the same setting, so a base captured from the player ends up handicapped as though the computer had held it from the start.

| Setting chosen | Slot the house holds while a person plays it | Slot after it passes to the computer |
| --- | --- | --- |
| Easy | 0, the `[Easy]` section | 2, the `[Difficult]` section |
| Normal | 1, the `[Normal]` section | 1, the `[Normal]` section |
| Hard | 2, the `[Difficult]` section | 0, the `[Easy]` section |

Outside a campaign every human house sits in slot 1 whatever the session was set to, so there the inversion leaves the house exactly where it was.

## The per-difficulty lists

`[General]` carries lists with one entry per difficulty. Every one of them is indexed with the raw slot and the inversion is left in place, so entry 0 is the hardest game setting and entry 2 the easiest.

[`TeamDelays`](/keys/teamdelays/), [`TotalAITeamCap`](/keys/totalaiteamcap/), [`MinimumAIDefensiveTeams`](/keys/minimumaidefensiveteams/), [`MaximumAIDefensiveTeams`](/keys/maximumaidefensiveteams/) and [`FillEarliestTeamProbability`](/keys/fillearliestteamprobability/) are read in every mode, and so are the twelve lists of the same shape that score the kinds of object the computer's Ion Cannon prefers. [`AIHateDelays`](/keys/aihatedelays/) and [`MultiplayerAICM`](/keys/multiplayeraicm/) are applied once, as a scenario outside a campaign finishes loading, and never in a campaign game.

[`MultiplayerAICM`](/keys/multiplayeraicm/) adds to what a computer house holds rather than replacing it. Its entry is a percentage of the house's credits plus the value of its stored Tiberium, and that percentage is handed to the house on top of what it already had, so an entry of `100` leaves it with twice its starting money and an entry of `0` changes nothing.

The team-creation countdown that `TeamDelays` sizes runs for every house, human or computer, and each house reads the list with its own slot. In a campaign game that puts the player's house and the computer's houses at opposite ends of one list. The table gives the entry each of them takes; the two columns are mirror images, and the entry the player's own house takes at the Easy setting is the entry a computer house takes at the Hard setting.

| Setting chosen | Entry a player-controlled campaign house reads | Entry a computer house reads |
| --- | --- | --- |
| Easy | 0 | 2 |
| Normal | 1 | 1 |
| Hard | 2 | 0 |

:::caution[The player's own house reads `TeamDelays` from the hard end]
Nothing about the countdown is restricted to computer houses. Lowering a campaign's difficulty moves the player's own house toward entry 0, so a map that switches that house into [the AI-trigger pass](/systems/ai-team-production/#when-the-pass-runs) runs the pass on the entry written for the hardest game setting.
:::

[AI triggers and team production](/systems/ai-team-production/#difficulty) covers what the AI lists do and why an AI trigger's difficulty flags mean the same thing in both modes while these lists do not.

## The computer's bonus with more than one human

Outside a campaign game, [`CompEasyBonus=yes`](/keys/compeasybonus/) drops a computer house one slot as it is assigned, provided the session holds more than one human entry and the house is not already in slot 0. A skirmish registers exactly one human entry, so the bonus never fires there; it takes a LAN or online session.

The name says easy and the step does the opposite. A computer house already holds the inverse of the setting the player chose, so dropping it a slot moves it to the slot the next harder setting would have given it. The table traces one computer house through both steps; the right-hand column is the setting its handicap then matches, and it is never easier than the one chosen.

| Setting chosen | Slot the computer house holds | Slot after the bonus | Setting it then behaves like |
| --- | --- | --- | --- |
| Easy | 2, the `[Difficult]` section | 1, the `[Normal]` section | Normal |
| Normal | 1, the `[Normal]` section | 0, the `[Easy]` section | Hard |
| Hard | 0, the `[Easy]` section | 0, the `[Easy]` section; no drop | Hard |

## What else the slot decides

Every row below reads a house's own slot directly, with no second inversion applied on top of it. So a row naming slot 0 or the `[Easy]` section describes what a computer house does when the player chose Hard, and a row naming slot 2 describes what it does when the player chose Easy.

| Where | What the slot changes |
| --- | --- |
| [The computer's base plan](/systems/ai-base-building/#building-the-plan) | The `2 - slot` extra refineries and the `3 - slot` term in the extra defense placeholders |
| [The GDI wall ring](/systems/ai-base-building/#walls-and-gates) | The `3 - slot` term in the cap on wall defenses |
| A computer house's harvesters | Outside a campaign, a house needs two harvesters per refinery on hand before it builds a replacement, or one when it sits in slot 2 |
| The computer's Ion Cannon | In slot 0 it also rates objects still under construction |
| Crushing an attacker | A computer house in slot 2 never answers fire by running the attacker down |
| A map's triggers | A trigger whose difficulty flags exclude the scenario's difficulty is disabled as it is created, and the enable-trigger action leaves it alone; outside a campaign the difficulty is the one the lobby's computer skill sets |

## Parsed settings without effect

The difficulty sections carry four settings no gameplay path reads, in three different states. [`BuildSlowdown`](/keys/buildslowdown/) has no reader at all. [`ContentScan`](/keys/contentscan/#scope-difficulty-settings) is compared in a routine nothing calls. [`Airspeed`](/keys/airspeed/#scope-difficulty-settings) and [`BuildDelay`](/keys/builddelay/) are folded into figures the house keeps, but no gameplay path reads those figures either. A country's own [`Airspeed=`](/keys/airspeed/#scope-housetype) is stored the same way and read no further. `[General]` carries [`FineDiffControl`](/keys/finediffcontrol/), and the [`ContentScan`](/keys/contentscan/#scope-global-rules) threshold in `[IQ]` is the other half of the same unreachable comparison.

The slot reaches nothing in a map's own triggers, which answer to the difficulty itself rather than to a house's slot. A trigger declares three per-difficulty fields and is disabled as it is created when the one for the scenario's difficulty is clear; [Trigger springing](/systems/trigger-springing/#difficulty) covers what that leaves it able to do. The three flags an AI trigger declares are a different set and do decide whether it is drawn.
