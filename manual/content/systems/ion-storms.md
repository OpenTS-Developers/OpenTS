---
title: Ion storms
summary: "Scripted global condition that grounds aircraft and hovercraft, calls lightning down on the map, and swaps the scenario to its ion lighting values."
category: superweapons-special
keys:
  - Ambient
  - AmbientChangeRate
  - AmbientChangeStep
  - Blue
  - Green
  - Ground
  - HunterSeeker
  - IonAmbient
  - IonBlue
  - IonGreen
  - IonGround
  - IonImmune
  - IonLevel
  - IonLightningDamage
  - IonLightningFrequency
  - IonLightningRandomness
  - IonRed
  - IonSensitive
  - IonStormWarhead
  - IonStormWarning
  - Level
  - LightningRod
  - LightningSound
  - MetallicDebris
  - Red
  - UseIonStorms
related:
  - type: action
    id: TACTION_ION_STORM_START
  - type: action
    id: TACTION_ION_STORM_STOP
  - type: action
    id: TACTION_ION_LIGHTNING_STRIKE
  - type: action
    id: TACTION_SET_AMBIENT_LIGHT
  - type: mission
    id: TMISSION_ION_STORM_START
  - type: mission
    id: TMISSION_ION_STORM_END
  - type: system
    id: emp-pulse
  - type: internal
    id: locomotion
---

One storm runs at a time, and it covers the whole map for every house. Every storm comes from scenario scripting. No rules setting, timer, or weather model starts one.

## Locomotors in brief

A storm grounds objects by cutting the power to their locomotor. Readers who already know locomotors can skip to [starting a storm](#starting-a-storm).

A **locomotor** is the part of an aircraft, vehicle, or infantryman that moves it. It decides whether the object drives, walks, hovers, flies, burrows, or steps. Each object gets the locomotor class that its type's [`Locomotor=`](/keys/locomotor/) names. That page lists the ten classes.

Only two classes are ion-sensitive: the flying locomotor, which aircraft use, and the hover locomotor, which hovercraft use. Objects on the drive, walk, jumpjet, tunnel, teleport, mech, levitate, and ballistic locomotors keep their power. Jumpjets follow [separate and harsher rules](#jumpjet-infantry).

On this page, "aircraft" means objects of a type listed in `[AircraftTypes]`. A vehicle type given the flying locomotor loses power in a storm like an aircraft, but the rules below that name aircraft do not apply to it.

## Starting a storm

### Trigger action

[Ion Storm start...](/mapping/actions/taction-ion-storm-start/) takes a number and starts a storm that lasts that many **seconds**. The action does nothing while a storm is already running.

### Team mission

[Ion storm start in...](/mapping/missions/tmission-ion-storm-start/) takes a number and starts a storm that lasts that many **game frames**. The team skips the mission while a storm is already running.

:::caution[The two scripted durations use different units]
The trigger action multiplies its number by 15 frames per second. The team mission uses its number as frames. `Ion Storm start...` with `20` gives a storm of 20 seconds, and `Ion storm start in...` with `20` gives a storm of 20 frames.
:::

A duration of `-1` never expires. Such a storm lasts until [Ion Storm stop...](/mapping/actions/taction-ion-storm-stop/) or [Ion storn end](/mapping/missions/tmission-ion-storm-end/) runs. "Ion storn end" is the engine's spelling of that mission's name.

Only the team mission can give a duration of `-1`, because the trigger action turns `-1` into `-15`. Any other duration of `0` or less ends the storm on the frame it breaks or the next one. Such a storm still has every effect of [the break](#the-storm-breaks), so the airborne aircraft it crashes stay lost.

### Random maps

[`UseIonStorms=yes`](/keys/useionstorms/) in a random map seed makes the generator load the file `ION.INI`. The generator applies that file's `[General]` section over the loaded rules and takes the six ion lighting values from its `[Lighting]` section. It also adds the file's trigger types and tag types to the generated map. Storms come only from those triggers; the option does not schedule any storm itself.

## The warning

Both the trigger action and the team mission start a countdown of [`IonStormWarning`](/keys/ionstormwarning/) seconds. The storm breaks when the countdown ends. With `IonStormWarning=0`, there is no countdown and the storm breaks on the frame the action or mission runs.

Each time the remaining time reaches a multiple of 15 seconds, EVA announces the approaching storm and an on-screen message shows for ten seconds. With `IonStormWarning=31`, the announcements come 30 and 15 seconds before the storm. A countdown of 15 seconds or less gives no announcement.

A second start request during the countdown does not restart it. The storm still breaks when the first countdown ends, but it lasts as long as the second request asked.

## The storm breaks

When the countdown ends, the engine does the following, in order:

1. Cuts the power of every object on the map with an ion-sensitive locomotor, and crashes the airborne aircraft among them. [Grounded locomotors](#grounded-locomotors) lists the objects that keep their power.
2. Makes [`IonAmbient`](/keys/ionambient/) the target of [the ambient fade](#the-ambient-ramp), and has the player's [radar](#radar) re-evaluated.
3. Stops the music and remembers which track was playing.
4. Tints every terrain palette, and the palette of every color scheme with more than one intensity level, with [`IonRed`](/keys/ionred/), [`IonGreen`](/keys/iongreen/), and [`IonBlue`](/keys/ionblue/). Screen static covers the tactical view while the palettes are rebuilt.
5. Starts the music track registered as `IONSTORM` and shows an ion storm message for ten seconds. EVA says nothing when the storm breaks; the countdown announcements are the only spoken warning.

:::caution[Register the storm music as IONSTORM]
[THEME.INI](/formats/theme-ini/) must register a track whose section is named `IONSTORM`, in any case. If no section has that name, the engine plays the first track whose `Name=` contains `IONSTORM` in exactly that case. If neither matches, the storm stops the music and plays nothing.
:::

## Lightning

### How often a bolt falls

Each frame of a storm calls a bolt with a chance of about [`IonLightningFrequency`](/keys/ionlightningfrequency/) percent. With `IonLightningFrequency=25`, a bolt is called on about one frame in four. Digits beyond the first decimal place are dropped.

### Where it strikes

[`IonLightningRandomness`](/keys/ionlightningrandomness/) is the percentage of bolts that strike a random cell. The other bolts are aimed at an object.

A random bolt strikes a random cell of the playfield. It can hit empty ground, including cells outside the playable area.

An aimed bolt first builds a list of candidates, then strikes the center cell of one candidate chosen with equal odds. If the list is empty, no bolt falls that frame. With `IonLightningRandomness=0`, a storm over a map with no candidates never strikes anything.

Aircraft are never candidates. Every building of any house can be one. A vehicle or infantryman of any house can be one under **any of** these conditions:

- it is not on a team, or its team's TeamType does not set [`IonImmune=yes`](/keys/ionimmune/);
- its type sets [`LightningRod=yes`](/keys/lightningrod/), which puts it back in the list despite `IonImmune`.

Each object that qualifies joins the list with the chance in this table. A lightning rod raises the chance only while the object holding it has power. A rod on a switched-off building, or on a vehicle or infantryman whose locomotor has lost power, gives the base 2%.

| Candidate | Chance of entering the list |
| --- | ---: |
| Any building, vehicle or infantryman not covered by a row below | 2% |
| Building with `LightningRod=yes` that is switched on | 42% |
| Vehicle or infantryman with `LightningRod=yes` whose locomotor still has power | 12% |
| Aircraft | never |

### What a strike does

A bolt strikes the ground of its cell, or the bridge deck when the cell has a bridge over it. At the strike point, the engine:

- plays [`LightningSound`](/keys/lightningsound/) at full volume, wherever on the map the bolt lands;
- plays the explosion animation that [`IonLightningDamage`](/keys/ionlightningdamage/) and [`IonStormWarhead`](/keys/ionstormwarhead/) select for the cell's land type;
- adds a flash of light when that warhead sets [`Bright=yes`](/keys/bright/);
- deals `IonLightningDamage` through `IonStormWarhead` to everything within the standard explosion radius of 1.5 cells. The bolt has no attacker, so no house gets credit for a kill;
- throws debris when the strike changed the cell, as described below;
- draws the bolt as a jagged line from the strike point up to a height of 200 levels.

The strike throws two to six animations picked from [`MetallicDebris`](/keys/metallicdebris/) under **any of** these conditions:

- the building in the cell after the blast is not the one that stood there before it;
- the vehicle, infantryman, aircraft, or building nearest the cell's north corner after the blast is not the one nearest it before. In a cell holding several objects, only that nearest one is compared;
- the cell's height changed;
- the cell held no building, vehicle, infantryman, or aircraft before the blast, and its land type is road, rock, wall, or weeds. Such a cell throws debris on every strike, even when nothing changes.

A vehicle, infantryman, or aircraft on a team whose TeamType sets `IonImmune=yes` takes no damage from the blast. Unlike the candidate list, this protection covers aircraft. It also covers any explosion whose warhead is the one `IonStormWarhead` names, not only lightning. `LightningRod=yes` does not remove this protection, so such a unit attracts bolts and survives them.

[Lightning strike at...](/mapping/actions/taction-ion-lightning-strike/) strikes its waypoint's cell in the same way, whether or not a storm is running.

## Battlefield effects

### Grounded locomotors

Each of the [two ion-sensitive locomotors](#locomotors-in-brief) has its own exemption.

An object on the flying locomotor keeps its power if its type sets [`HunterSeeker=yes`](/keys/hunterseeker/).

An object on the hover locomotor keeps its power under **any of** these conditions:

- it is in radio contact with a building whose type sets [`WeaponsFactory=yes`](/keys/weaponsfactory/);
- it stands on the first, third, or fourth cell of the second row of a `WeaponsFactory=yes` building's foundation.

These exemptions keep a storm from stranding a newly built hovercraft in the factory doorway. An exempt hovercraft keeps its power until it receives a move order during the storm while neither condition holds. It loses power when it receives that order.

An object whose locomotor has lost power cannot move, the same result [an EM pulse](/systems/emp-pulse/#what-a-pulse-reaches) has on movement. A storm does not stun, so a grounded hovercraft can still fire.

A moving aircraft tumbles, stops, and sinks. A hovercraft drops its move order, drifts as it settles, and comes to rest tilted to the slope of its cell.

An object with an ion-sensitive locomotor that is placed on the map during a storm, such as a newly built aircraft, arrives without power unless an exemption applies.

### Aircraft

An aircraft that loses power while off the ground crashes. Its strength drops to zero, its passengers die, and it tumbles down. No house gets credit for the kill. An aircraft already on the ground only loses power.

For the rest of the storm, no aircraft can fire, whatever its weapons. Parking on a repair or reload building does not restore an aircraft's power. Lightning never aims at aircraft.

### Jumpjet infantry

The jumpjet locomotor is not ion-sensitive, but a storm destroys a jumpjet that moves while it is off the ground. Such a jumpjet takes damage equal to its current strength through the [`C4Warhead`](/keys/c4warhead/) warhead in `[CombatDamage]`. Armor does not reduce this damage, and no house gets credit for the kill. A jumpjet hovering in place with no move order survives until it is ordered to move.

A jumpjet on the ground cannot take off during a storm. Move orders route surviving jumpjets on foot, like ordinary infantry.

### Radar

The player loses the radar for the whole storm, whatever their power and radar buildings. When the storm ends, the radar returns if [power and a radar building](/systems/power/#radar) would otherwise provide it. A player given the whole map keeps the radar during a storm; [observers and coach mode](/systems/observers/) owns that rule.

### Weapons, production, and repair

- A weapon with [`IonSensitive=yes`](/keys/ionsensitive/) cannot fire during a storm.
- A factory that finishes an aircraft during a storm places it on the ground on a nearby cell instead of docking it.
- A service depot does not restore power to a docked vehicle during a storm, and neither does a repair building the vehicle stands on.

## Lighting

A scenario's `[Lighting]` section holds two sets of values: the ordinary [`Ambient`](/keys/ambient/), [`Red`](/keys/red/), [`Green`](/keys/green/), [`Blue`](/keys/blue/), [`Ground`](/keys/ground/), and [`Level`](/keys/level/#scope-scenarios), and their `Ion` counterparts. A map writes both sets in the same section:

```ini title="map file"
[Lighting]
Ambient=0.870000    ; the level the map fades back to when a storm ends
Red=1.000000        ; the tint the map uses the rest of the time
Green=1.000000
Blue=1.000000
Ground=0.000000     ; ground darkening while no storm runs
Level=0.000000      ; height shading while no storm runs
IonAmbient=0.500000 ; the level the storm fades the map toward
IonRed=1.620000     ; the tint the storm lays over every palette
IonGreen=1.250000
IonBlue=0.340000
IonGround=0.000000  ; ground darkening while the storm runs
IonLevel=0.000000   ; height shading while the storm runs
```

While a storm runs, cell brightness uses [`IonLevel`](/keys/ionlevel/) and [`IonGround`](/keys/ionground/) in place of `Level` and `Ground`. Aircraft also take their height brightness from `IonLevel`, and so do vehicles and infantry above the ground. A palette created during the storm, such as one for a newly lit cell, gets the ion tint at once.

The tint and the light level change on different schedules. The tint switches when the storm breaks and switches back when it ends. The light level fades toward `IonAmbient` through [the ambient ramp](#the-ambient-ramp), and fades back to `Ambient` after the storm.

An omitted ion key takes the value of the ordinary key it pairs with, so a map that sets only `Ambient`, `Red`, `Green`, and `Blue` gets matching ion values. The ground and level keys are the exception.

:::caution[Set IonGround and IonLevel explicitly]
When `Ground`, `Level`, `IonGround`, or `IonLevel` is omitted, its fallback value is rounded down to a whole number, so any fraction becomes `0`. An omitted `Ground` or `Level` is therefore `0`. An omitted `IonGround` or `IonLevel` copies `Ground` or `Level` rounded down, which is `0` unless that key is `1` or more. A value written to any of the four keys is kept as written. To have ground darkening or height shading during a storm, set `IonGround` and `IonLevel`.
:::

### The ambient ramp

While the current light level differs from its target, the engine moves it toward the target by [`AmbientChangeStep`](/keys/ambientchangestep/), in the same units as `Ambient`. It then waits [`AmbientChangeRate`](/keys/ambientchangerate/) minutes, at 900 frames to the minute, before the next step. A step never passes the target. Both the storm's darkening and the return to daylight use this ramp.

:::caution[Keep AmbientChangeRate above 0]
With `AmbientChangeRate=0`, the light level never fades. A storm then tints the map but never darkens it, and the [Set ambient light...](/mapping/actions/taction-set-ambient-light/) action has no visible effect.
:::

[Ambient light <= ...](/mapping/events/tevent-ambient-less-than/) and [Ambient light >= ...](/mapping/events/tevent-ambient-greater-than/) are tested only after a fade step. A storm's darkening and its return to daylight each produce a series of such steps.

`Set ambient light...` stores its new level at once. While a storm runs, the map does not fade to it; the map fades to that level when the storm ends. [Set ambient rate...](/mapping/actions/taction-set-ambient-rate/) and [Set ambient step...](/mapping/actions/taction-set-ambient-step/) change the ramp's rate and step for the rest of the scenario, whether or not a storm is running.

## The storm ends

A storm ends when its duration runs out, or when `Ion Storm stop...` or `Ion storn end` runs. Neither of those stops a storm that is still in its warning countdown, so that storm breaks as scheduled. When a storm ends, the engine:

1. Restores power to every object with an ion-sensitive locomotor, including those in [limbo](/glossary/#limbo) that the break skipped.
2. Makes the scenario's ambient level the target of the ambient fade, so the map fades back instead of snapping. That level is `Ambient`, or the level a `Set ambient light...` action stored during the storm.
3. Has the player's radar re-evaluated.
4. Stops the storm music and restarts, from its beginning, the track that was playing when the storm broke.
5. Removes the ion tint from every terrain palette and color scheme, again behind screen static.

Losses are permanent. Crashed aircraft and destroyed jumpjets do not come back. Nothing schedules another storm; it needs another trigger action or team mission.

## Settings the engine ignores

Two settings look like ion storm controls but have no effect:

- [`IonStormDuration`](/keys/ionstormduration/) in `[General]` is not a default storm length. Every storm lasts as long as its trigger action or team mission says.
- [`IonStorms`](/keys/ionstorms/) in `[SpecialFlags]` does not enable or disable storms. Scripted storms run whether it is set or cleared.

The ion storm crate result starts no storm. [Crates](/systems/crates/#settings-and-results-without-effect) owns that result.
