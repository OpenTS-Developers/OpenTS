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

One storm exists at a time and it covers the whole map for every house. Nothing raises one on its own: no rules setting, timer, or weather model schedules a storm, so every storm in a scenario comes from scenario scripting.

## Locomotors in brief

This section introduces the entity the rest of the page turns on. Anyone who already knows what a locomotor is can skip to [starting a storm](#starting-a-storm).

A **locomotor** is a separate object that moves one aircraft, infantryman or vehicle about. It settles whether that object drives, walks, hovers, flies, burrows or steps, and it is what an ion storm switches off underneath it. Each instance is given one as it is created, of the class its type's [`Locomotor=`](/keys/locomotor/) names; ten classes are registered, and that page lists them.

Only two of the ten report themselves sensitive to an ion storm: the flying locomotor, which aircraft use, and the hover locomotor, which hovercraft use. Drive, walk, jumpjet, tunnel, teleport, mech, levitate and ballistic objects keep their power throughout, and the jumpjet is [handled separately and more harshly](#jumpjet-infantry).

A storm therefore reaches an object through its locomotor rather than through what kind of object it is. That is why aircraft and hovercraft are named together below wherever a locomotor decides the outcome, and vehicles, infantry and aircraft are named together wherever something else does.

## Starting a storm

### Trigger action

[Ion Storm start...](/mapping/actions/taction-ion-storm-start/) takes a number and starts a storm that lasts that many **seconds**. The action is refused outright while a storm is already running.

### Team mission

[Ion storm start in...](/mapping/missions/tmission-ion-storm-start/) takes a number and starts a storm that lasts that many **game frames**. It is skipped while a storm is already running.

:::caution[The two scripted durations use different units]
The trigger action multiplies its number by the frame rate of 15 frames per second; the team mission passes its number through unchanged. `Ion Storm start...` with `20` runs for 20 seconds, and `Ion storm start in...` with `20` runs for 20 frames.
:::

A duration of `-1` is the one value that never expires; a storm started with it lasts until [Ion Storm stop...](/mapping/actions/taction-ion-storm-stop/) or [Ion storn end](/mapping/missions/tmission-ion-storm-end/) (the engine's own spelling of that mission's name) runs. Only the team mission can deliver that value unchanged.

### Random maps

[`UseIonStorms=yes`](/keys/useionstorms/) in a map seed makes the generator load the hard-coded file `ION.INI` while it builds the map. The generator applies that file's `[General]` section over the loaded rules, reads the six ion values from its `[Lighting]` section, and registers that file's trigger types and tag types with the generated map. Storms then come from whatever triggers the file holds, because the engine never schedules one itself. When the generator randomizes a seed itself, it turns the option on for about half of all seeds, and only while the Firestorm addon is enabled.

## The warning

Both scripted entry points schedule the storm rather than breaking it at once: they arm a countdown of [`IonStormWarning`](/keys/ionstormwarning/) seconds, which then runs down one frame at a time. Every fifteen seconds of that countdown, the EVA line for an approaching ion storm plays and an on-screen message holds for ten seconds. At the default of 31 seconds the warning is therefore announced twice, 30 and 15 seconds out. A warning of fifteen seconds or less never reaches a multiple, so no announcement plays. Setting `IonStormWarning=0` breaks the storm on the frame the action or mission runs.

A second start request during the countdown keeps the shorter of the two countdowns, but replaces the pending duration outright, so the storm arrives on the earlier schedule and runs for the later request's length.

## The storm breaks

When the countdown reaches zero the engine, in order:

1. Cuts the power to every aircraft and hovercraft that is on the map and whose locomotor is one of the two ion-sensitive ones, and crashes the aircraft among them.
2. Sets the desired ambient light to [`IonAmbient`](/keys/ionambient/) and marks the player's radar for re-evaluation.
3. Remembers which music track is playing and stops it.
4. Retints every terrain lighting table, and the shape remap table of every color scheme with more than one intensity level, to [`IonRed`](/keys/ionred/), [`IonGreen`](/keys/iongreen/), and [`IonBlue`](/keys/ionblue/). Screen static is tiled over the tactical view while those palettes are rebuilt.
5. Starts the music track registered as `IONSTORM` and posts an on-screen ion-storm message for ten seconds. No EVA line accompanies the break itself. The approach warnings are the only spoken cue.

:::caution[The storm music track must be registered under that exact name]
The engine looks the storm track up by the name `IONSTORM`: first matched case-insensitively against registered music filenames, then case-sensitively as a substring of their full names. [THEME.INI](/formats/theme-ini/) registers the track. When no registered track matches, the break stops whatever was playing and starts nothing.
:::

## Lightning

### How often a bolt falls

Every frame of a storm draws one number from `0` through `1000` inclusive and calls a bolt when it falls below ten times [`IonLightningFrequency`](/keys/ionlightningfrequency/). At the default of `25` that is a bolt on roughly one frame in four. The multiplication is part of how the value is read, so the figure written in `[General]` is a tenth of the threshold actually compared.

### Where it strikes

[`IonLightningRandomness`](/keys/ionlightningrandomness/) is the percentage of bolts that land on a random cell. The remainder are aimed at an object.

A random bolt draws a cell from the map rectangle, the upright square of cells that encloses the playfield. It redraws until the cell lies inside the playfield, so a bolt never falls on a cell the map does not have.

An aimed bolt builds a candidate list by walking every active object on the map, regardless of house. Aircraft are dropped at once. Each of the rest is put through one exemption and then rolled against a chance that its own kind and state decide.

An object is entered into the candidate list under **any of**:

- it is a building, which is never on a team at all;
- it is on no team, or its team's TeamType is not [`IonImmune=yes`](/keys/ionimmune/);
- its own type sets `LightningRod=yes`, which overrides `IonImmune` whatever the TeamType says.

The table then gives the chance each survivor is rolled against. What to read off it is how much a lightning rod is worth and where it stops being worth anything. A switched-on building has twenty-one times the base chance and a moving object six times it, but the rod is worth nothing at all once the thing holding it goes dark. A rod on a switched-off building and a rod on a vehicle or infantryman whose locomotor has lost power both fall back to the base figure rather than dropping the object out.

| Candidate | Chance of entering the list |
| --- | ---: |
| Any building, vehicle or infantryman not covered by a row below | 2% |
| Building with [`LightningRod=yes`](/keys/lightningrod/) that is switched on | 42% |
| Vehicle or infantryman with `LightningRod=yes` whose locomotor still has power | 12% |
| Aircraft | dropped before any of the above is tested |

One entry is then drawn from the finished list with equal probability and its center cell is struck. An empty list produces no bolt on that frame, so a storm over a map with no objects and `IonLightningRandomness=0` never strikes anything.

### What a strike does

The strike point is the cell's ground level, raised by the bridge height when the cell lies under a bridge. At that point the engine:

- plays [`LightningSound`](/keys/lightningsound/) without a position, so the clap is at full volume wherever on the map the bolt lands;
- creates the combat animation selected by [`IonLightningDamage`](/keys/ionlightningdamage/), [`IonStormWarhead`](/keys/ionstormwarhead/), and the cell's land type;
- adds a spotlight flash when that warhead sets [`Bright=yes`](/keys/bright/);
- applies `IonLightningDamage` through `IonStormWarhead` across the standard 1.5-cell explosion radius, with no source recorded, so no house is credited with a kill;
- throws between two and six animations drawn from [`MetallicDebris`](/keys/metallicdebris/) under **any of**:
  - the building standing in the cell after the blast is not the one that stood there before it;
  - the vehicle, infantryman or aircraft in the cell is likewise not the one that was there before it;
  - the cell's terrain height changed;
  - the cell held neither a building nor any such object to begin with and its land type is road, rock, wall or weeds. This last term asks for no change at all, so an empty stretch of road throws debris on every strike;
- draws the bolt itself as a chain of laser segments climbing from the strike point to 200 height levels, each segment displaced by up to 128 leptons horizontally.

A vehicle, infantryman or aircraft on a team whose TeamType sets `IonImmune=yes` takes no damage from the blast. Unlike the candidate list above, this test does reach aircraft. It is also keyed to the warhead rather than to the storm: any weapon configured with the same warhead as `IonStormWarhead` skips such objects in the same way.

[Lightning strike at...](/mapping/actions/taction-ion-lightning-strike/) calls this same routine at its waypoint cell. It is independent of the storm state and works whether or not a storm is running.

## Battlefield effects

### Grounded locomotors

Each of the [two ion-sensitive locomotors](#locomotors-in-brief) has its own exemption, and the two are not the same shape.

The flying one has a single term: an object on the flying locomotor whose type sets [`HunterSeeker=yes`](/keys/hunterseeker/) is never sensitive.

The hover one is two separate exemptions rather than one two-part test, and either alone spares the object. An object on the hover locomotor is exempt under **any of**:

- it is in radio contact with a building whose type is [`WeaponsFactory=yes`](/keys/weaponsfactory/);
- it stands on one of a `WeaponsFactory=yes` building's own doorway cells, in the second row of that building's foundation at zero, two, or three cells across.

Either way a storm cannot strand a newly built vehicle in the doorway. The exemption is re-tested rather than remembered. A hover object handed a move order while a storm is running puts itself through both terms again on the spot, and cuts its own power there and then if neither still holds.

An unpowered locomotor moves its object nowhere, which is also part of what [an EM pulse](/systems/emp-pulse/#what-a-pulse-reaches) does. A storm stops there: it does not stun, so a grounded hovercraft may still fire. An aircraft that was moving tumbles, stops, and sinks; a hovercraft abandons its move order, drifts as it settles, and comes to rest on the slope of the cell beneath it. An object created during a storm arrives with its locomotor already off if that locomotor is ion-sensitive.

### Aircraft

Cutting the power to a sensitive flying locomotor also puts the aircraft through the crash path. One that is off the ground at all has its strength set to zero with no kill credited, kills its cargo, and starts tumbling. One that was already on the ground is only powered off, because the crash path does nothing at zero height.

For the rest of the storm no aircraft may fire at all, whatever its weapons, and a repair or reload building an aircraft is parked on will not restore its power. Aircraft are also skipped when lightning picks a target.

### Jumpjet infantry

The jumpjet locomotor is not ion-sensitive and keeps its power, but a storm handles it separately and more harshly. A jumpjet that is moving and not on the ground takes damage equal to its whole current strength through the `[CombatDamage]` warhead [`C4Warhead`](/keys/c4warhead/) with no source, which destroys it. Its locomotor ends that frame as soon as the damage destroys or otherwise removes the infantryman, without taking another flight-state step. A grounded jumpjet cannot take off: it never leaves its grounded flight state, and the decision to make a trip by air is refused for the duration. Move-order resolution demotes a flyer movement zone to the infantry movement zone while a storm runs, so surviving jumpjets route on foot.

:::caution[Airborne jumpjets die where airborne aircraft crash]
Both are taken out of the air, but a crashing aircraft goes through the ordinary crash path and a jumpjet is killed by direct damage.
:::

### Radar

The player's house loses radar outright for the duration. The radar availability pass refuses the radar while a storm is running, before it looks at power or at radar buildings, and switches the map's radar off. Only the player's own house is evaluated, and coverage is recalculated on the first house pass after the storm ends. A player who has been given the whole map keeps the radar through the storm; [observers and coach mode](/systems/observers/) owns that rule.

### Weapons, production, and repair

- A weapon with [`IonSensitive=yes`](/keys/ionsensitive/) cannot fire for the duration.
- A factory that finishes an aircraft during a storm places it on a nearby free cell instead of docking it.
- A service depot does not power a docked vehicle back on during a storm, and neither does a repair building the vehicle is standing on.

## Lighting

A scenario's `[Lighting]` section holds two complete sets of values: the ordinary [`Ambient`](/keys/ambient/), [`Red`](/keys/red/), [`Green`](/keys/green/), [`Blue`](/keys/blue/), [`Ground`](/keys/ground/), and [`Level`](/keys/level/#scope-scenarios), and their `Ion` counterparts. While a storm is active, cell brightness is computed from [`IonLevel`](/keys/ionlevel/) and [`IonGround`](/keys/ionground/) in place of `Level` and `Ground`. The height bonus drawn onto aircraft and onto elevated vehicles and infantry uses `IonLevel` as well. Any lighting table built while the storm runs — the terrain one a newly lit cell asks for, or a color scheme's shape remap table — is created with the ion tint already applied.

A map writes both sets into the same section:

```ini title="map file"
[Lighting]
Ambient=0.870000    ; the level the map fades back to when a storm ends
Red=1.000000        ; the tint the map uses the rest of the time
Green=1.000000
Blue=1.000000
Ground=0.000000     ; ground darkening while no storm runs
Level=0.000000      ; height shading while no storm runs
IonAmbient=0.500000 ; the level the storm fades the map toward
IonRed=1.620000     ; the tint the storm lays over every lighting table
IonGreen=1.250000
IonBlue=0.340000
IonGround=0.000000  ; ground darkening while the storm runs
IonLevel=0.000000   ; height shading while the storm runs
```

The two halves of the change do not run on the same clock. The palette tint is applied outright when the storm breaks and reversed outright when it ends. The ambient level is only a target: the storm sets the desired level to `IonAmbient`, and the ramp below takes the current level toward it over time.

Each ion key falls back to the ordinary key read earlier in the same section, so a map that sets only `Ambient`, `Red`, `Green`, and `Blue` gets ion values to match. The ground and level keys are the exception.

:::caution[The four ground and level keys collapse to zero when omitted]
The fallback for `Ground`, `Level`, `IonGround`, and `IonLevel` divides two whole numbers, so any fraction below `1` truncates to `0`. Omitting `Ground` or `Level` stores `0` rather than the engine's own starting fractions. An omitted `IonGround` or `IonLevel` takes the ordinary key read just before it, truncated the same way, so an ordinary key set below `1` leaves its ion key at `0`. The truncation is in the fallback alone: a fraction written to `IonGround` or `IonLevel` itself is kept. A scenario that wants ground darkening or height shading during a storm has to state `IonGround` and `IonLevel` outright.
:::

### The ambient ramp

Whenever the current ambient level differs from the desired one, the engine waits [`AmbientChangeRate`](/keys/ambientchangerate/) minutes, at 900 frames to the minute. It then moves the current level by [`AmbientChangeStep`](/keys/ambientchangestep/) times 100, clamped so it never passes the target. The storm's darkening and the return to daylight both travel this way.

:::caution[A rate of zero freezes the ambient level]
The ramp is guarded on a non-zero `AmbientChangeRate`. At `0` the current ambient level never moves again, so a storm retints the palette but never darkens the map, and the scripted [Set ambient light...](/mapping/actions/taction-set-ambient-light/) action never takes visible effect either.
:::

Every completed step raises the ambient-changed flag, which is what admits [Ambient light <= ...](/mapping/events/tevent-ambient-less-than/) and [Ambient light >= ...](/mapping/events/tevent-ambient-greater-than/) to that frame's trigger evaluation. Those events are therefore tested only while a fade is stepping, and a storm's darkening and its return to daylight are each a run of such frames.

`Set ambient light...` stores its new level immediately but withholds the fade while a storm is running; the map travels to it once the storm clears. [Set ambient rate...](/mapping/actions/taction-set-ambient-rate/) and [Set ambient step...](/mapping/actions/taction-set-ambient-step/) overwrite the ramp's rate and step in the loaded rules, and those overwrites outlive the storm.

## The storm ends

A storm ends when its duration elapses, or when `Ion Storm stop...` or `Ion storn end` runs. The engine then:

1. Restores the power of every aircraft and hovercraft, including the ones in [limbo](/glossary/#limbo) that the opening pass skipped.
2. Sets the desired ambient light back to `Ambient`, so the map fades home rather than snapping.
3. Marks the player's radar for re-evaluation.
4. Stops the storm music track and resumes the one that was playing when the storm broke.
5. Returns every terrain lighting table and every color scheme's shape remap table to its remembered tint, again behind screen static.

Losses are permanent. Aircraft that crashed and jumpjets that were destroyed do not come back, and nothing schedules a repeat: another storm needs another trigger action or team mission.

## Settings the engine ignores

Two settings look like ion-storm controls and are read into the engine, but nothing reads them. [`IonStormDuration`](/keys/ionstormduration/) in `[General]` is not a default storm length. Every storm's length comes from the number its trigger action or team mission names. [`IonStorms`](/keys/ionstorms/) in `[SpecialFlags]` does not gate storms; scripted storms run whether it is set or cleared. The ion-storm crate result is inert in the same way: drawing it consumes the crate and plays that row's own animation, and no storm starts. [Crates](/systems/crates/#settings-and-results-without-effect) owns that result.
