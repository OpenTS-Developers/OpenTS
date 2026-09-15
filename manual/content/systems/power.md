---
title: Power output and drain
summary: "Tallies each house's power output against its drain and degrades its base while the balance falls short."
category: buildings-economy
keys:
  - BuildSpeed
  - BuildTime
  - C4Warhead
  - CloakGenerator
  - CloakRadiusInCells
  - ConditionYellow
  - DamageDelay
  - FreeRadar
  - Immune
  - IsPowered
  - LaserFencePost
  - MessageDelay
  - MinDamage
  - MinProductionSpeed
  - MultipleFactory
  - Power
  - Powered
  - PowersUpBuilding
  - PowersUpToLevel
  - Radar
  - SensorArray
  - SpeakDelay
  - TogglePower
  - Upgrades
  - Verses
  - WallBuildSpeedCoefficient
related:
  - type: system
    id: emp-pulse
  - type: system
    id: ai-base-building
  - type: event
    id: TEVENT_LOW_POWER
  - type: action
    id: TACTION_TURN_OFF_ATTACHED
  - type: action
    id: TACTION_TURN_ON_ATTACHED
---

Every house holds two figures: the power its structures produce and the power they consume. Every consequence on this page is read from the ratio between them. Both figures are rebuilt from zero rather than adjusted as structures come and go.

## How the balance is computed

A rebuild walks every structure in the game and keeps the ones this house owns that are out of [limbo](/glossary/#limbo) and standing on the map, then adds each one's output and drain. A structure counts from the moment it is placed, so one still playing its buildup animation is already on both sides of the ledger.

In a campaign game a house under the player's control skips any of its own structures the player has not discovered yet, so a base handed over by a trigger contributes nothing until it is seen. Discovering an object flags the tally for a rebuild.

### Writing the assignments

A producer, a consumer, a plug and the global settings that govern a shortfall are written like this:

```ini title="rules.ini"
[MYPOWER]        ; example BuildingType that supplies the grid
Power=100
Upgrades=1       ; upgrade slots this kind of structure offers

[MYSAM]          ; example BuildingType that draws from it
Power=-50

[MYTURBINE]      ; example plug, fitting the slot above
PowersUpBuilding=MYPOWER
Power=50         ; the plug's own contribution, added before the scaling

[General]
DamageDelay=2            ; game minutes between damage ticks while short
MinProductionSpeed=0.75  ; floor under the production multiplier
```

### What each structure contributes

Output is the type's [`Power=`](/keys/power/#scope-buildingtype) plus the value of every plug the structure holds. A plug is a structure type with [`PowersUpBuilding=`](/keys/powersupbuilding/), installed into one of the host's [upgrade slots](/keys/upgrades/). That sum is scaled by the structure's current strength as a fraction of its maximum, and truncated to a whole number. Drain is the same sum over the type and its plugs, and is not scaled at all.

The table gives each contributor and each condition against its effect on the two sides, and a stun there is an [EM pulse](/systems/emp-pulse/) holding the structure down for a set number of frames. Only one row tells the two columns apart, and that row decides what a base under fire loses.

| | Output | Drain |
| --- | --- | --- |
| Type | positive `Power=` | magnitude of a negative `Power=` |
| Plugs | added before scaling | added |
| Damage | scales the total, truncated | ignored |
| Switched off | nothing | nothing |
| Stunned | unchanged | unchanged |

Three consequences follow. Damage costs output from the first point taken, with no threshold under it: a plant rated 100 with 999 strength of 1000 supplies 99, and the same plant at 1 strength supplies nothing at all. Drain never falls with damage, so a base being shelled loses supply while demand holds steady and slips into low power. Only the on/off switch takes a structure off either side, and an EM pulse never touches that switch. A stunned plant therefore keeps feeding the grid, and a stunned consumer keeps drawing.

### When the tally is rebuilt

The tally is flagged as stale rather than recomputed on the spot, and the rebuild runs on the house's next turn; anything that could move either figure sets the flag. That covers a structure's strength changing, opening for business, being taken off the map, changing hands, having a plug installed or sold, being switched on or off, and being discovered.

The rebuild then brings the house's structures into line with the new balance, re-rates its factories and marks its radar for re-evaluation, and refreshes the house's superweapons when the rebuild took the house across the full-power line.

Outside campaign games a computer house answers a shortfall in its own way, by inserting a power plant ahead of the node that would have caused it; [AI base planning](/systems/ai-base-building/#power-and-money-interventions) covers that and the margin it works to.

## The power fraction

The fraction is 1 whenever output is at least drain, and also 1 for a house with no drain at all however little that house produces. Below that the fraction is output divided by drain, and 0 when output is zero.

:::caution[Surplus buys nothing]
The fraction is never above 1. A house at twice its drain is in the same state as a house at exactly its drain, so extra plants are insurance against losing one, not an upgrade.
:::

## Switching a structure off

Power mode is entered from [Power Mode](/commands/togglepower/) or the sidebar's power button. Its cursor offers the toggle only over an object that clears all of these, in this order:

- it belongs to a player-controlled house;
- it is a structure;
- its type is [`Selectable=yes`](/keys/selectable/);
- its type is not a deployed vehicle (one with [`UndeploysInto=`](/keys/undeploysinto/) without being [`ConstructionYard=yes`](/keys/constructionyard/));
- its type is [`TogglePower=yes`](/keys/togglepower/);
- **Any of:** its type sets drain, or its type is [`Powered=yes`](/keys/powered/).

The toggle order the cursor queues refuses a [`FirestormWall=yes`](/keys/firestormwall/) type outright.

The switch itself asks less than the cursor does. It needs only that the type drains power or is `Powered=yes`. A [Turn off building](/mapping/actions/taction-turn-off-attached/) trigger action or a scenario's own structure record can therefore put a `TogglePower=no` structure out of service, and the player can never switch it back. Switching a structure on is refused while it is stunned.

Taking a structure out of service removes it from both sides of the tally, disables its light source, collapses its cloaking field, drops its laser fences, and stops its powered animations. It announces the change to a player-controlled house, and when the structure is a factory it re-examines what its house may still produce. Switching off the house's last working firestorm generator also discharges an active firestorm wall. While a structure is off and its house is player-controlled, a marker is drawn over it wherever the cell is neither shrouded nor fogged.

## What low power costs

Everything below reads the same fraction, and applies to every house, the computer's included, except where noted.

### The structure damage tick

Each house holds a timer reloaded with [`DamageDelay`](/keys/damagedelay/) game minutes. When it expires and the house's fraction is below 1, 1 point of damage goes through [`C4Warhead`](/keys/c4warhead/) to every structure that clears all of these:

- the house owns it;
- its strength stands strictly above [`ConditionYellow`](/keys/conditionyellow/) of its maximum;
- its type sets drain of its own.

The timer is reloaded whether or not the house was short, so the cadence never drifts.

Those three tests are the whole filter. A structure that has been switched off still ticks, although it has already stopped adding to the drain that caused the shortfall. The test is on the type's own drain, so a structure that draws power only through its plugs is never touched. And because only structures above `ConditionYellow` are hit, a shortfall grinds a base down to that threshold and stops there. At the stock one-point tick it cannot destroy anything, though a raised [`MinDamage`](/keys/mindamage/) can overshoot the threshold and, once it exceeds a structure's remaining strength, destroy it outright.

:::caution[Verses cannot soften the tick]
The point of damage is not forced, so it goes through the warhead's [`Verses`](/keys/verses/) table. At a raw damage of 1 no percentage below 200 can reduce it, and the result is then raised to [`MinDamage`](/keys/mindamage/). Only 200 percent and above multiplies the tick, and raising `MinDamage` raises it for every structure at once. [`Immune=yes`](/keys/immune/) is the way out.
:::

### Production

The fraction is turned into a production multiplier on a fixed ladder. Each row of the table is a band of the fraction and the multiplier that band produces. The bottom row settles that the multiplier never falls below 0.5, so even a house producing nothing at all still builds at half speed.

| Power fraction | Production multiplier |
| --- | --- |
| 1 | 1 |
| 0.75 up to but not including 1 | 0.75 |
| 0.5 up to but not including 0.75 | the fraction itself |
| below 0.5 | 0.5 |

The multiplier is then raised to [`MinProductionSpeed`](/keys/minproductionspeed/) when it falls below it, and the object's build time is divided by the result, where 0.75 makes a build a third longer and 0.5 doubles it.

:::caution[The middle band is the only one that tracks the shortfall]
Between half and three quarters the multiplier is the raw fraction, so a house at 0.6 builds slower than one at 0.74. Above and below that band the penalty is flat. At the default `MinProductionSpeed` the floor sits exactly on the bottom step, so it changes nothing until it is moved; raising it lifts the whole ladder at once.
:::

Build time reaches that division already shaped. It starts from the object's cost through [`BuildSpeed`](/keys/buildspeed/) and is multiplied by the country and difficulty [`BuildTime`](/keys/buildtime/) figures the house was handed. After the power division it is adjusted for the number of factories through [`MultipleFactory`](/keys/multiplefactory/) and, for a wall, by [`WallBuildSpeedCoefficient`](/keys/wallbuildspeedcoefficient/).

Every change to the balance re-rates the house's factories. Each one recomputes the delay between production steps from the new build time but keeps the step it had already reached, so low power slows a build in place instead of restarting it. Restoring power speeds it up again from where it stands. That delay is a whole number of frames between 1 and 255, so once a step already takes 255 frames a deeper shortfall costs nothing further.

What a shortfall never does is remove an option. Whether a house may build something is decided by whether it owns a switched-on factory of the right kind; the power balance is not read. Low power therefore leaves every cameo on the sidebar and only slows what comes out of it.

### Radar

Only the local player's house has a radar map to lose. It is raised while all of these hold:

- no ion storm is running;
- the house's output is at least its drain (the raw comparison, not the fraction);
- **Any of:**
  - the scenario sets [`FreeRadar=yes`](/keys/freeradar/);
  - the house owns a [`Radar=yes`](/keys/radar/) structure that is **All of:** switched on, out of limbo, on the map, and not being deconstructed.

The campaign discovery skip applies to that search too. A player who has been given the whole map keeps the radar whatever these tests say; [observers and coach mode](/systems/observers/) owns that rule.

:::caution[The search stops at the first radar it finds]
Structures are scanned in creation order and the scan ends at the first eligible one, which supplies the radar only if it is not stunned. A stunned radar reached first therefore keeps the map dark while a second, working one stands beside it.
:::

### Superweapons

A superweapon that depends on a building counts as enabled while a structure granting it is switched on, whether the grant comes from that structure's own type or from a plug. It is disabled outright whenever the house's output is below its drain. A disabled weapon is suspended only if its type is [`IsPowered=yes`](/keys/ispowered/); the resume step does not read that value, so a weapon suspended some other way still comes back.

Suspension stops the charge timer where it stands, replaces the sidebar cameo's status text with the engine's hold caption, and refuses the targeting cursor. A weapon that becomes available while its house is already short of power arrives suspended. A weapon granted by the [Add repeating special weapon](/mapping/actions/taction-full-special/) trigger action no longer needs a building and is never suspended by this path.

:::caution[A charge-draining weapon loses its charge, not just its time]
Where an ordinary weapon resumes from the point its timer stopped, a [`UseChargeDrain=yes`](/keys/usechargedrain/) weapon has its timer reset to a full [`RechargeTime`](/keys/rechargetime/) on resume. Each spell of low power, however brief, therefore costs it every second of charge it had accumulated.
:::

### Defenses

Three separate tests stand between a shortfall and a silent defense, and they do not agree with each other.

1. **Out of service.** A structure is **operational** while none of these holds:

   - it is switched off;
   - it is stunned;
   - its strength has reached zero;
   - **All of:** its type is `Powered=yes`, its type sets drain of its own, its type is `TogglePower=yes`, and its house's power fraction is below 1.

   That is the first gate on firing, and the test used by spotlights, [laser fences](/systems/laser-fences/), sensor-array refreshes and cloak-generator regrowth.

2. **Weapons.** Past that gate, a structure whose type is `Powered=yes` with drain reports itself busy while the fraction is below 1. This test omits `TogglePower`, so a `Powered=yes`, `TogglePower=no` defense is silenced even though the first test spared it.

3. **SAM tracking.** A [`SAM=yes`](/keys/sam/) launcher stalls in its ready state under the same `Powered`/drain/fraction test, so it never turns toward its target in the first place.

A structure whose primary weapon is an electric weapon charges only while it holds a target, its house is at full power, and it is switched on. That branch reads neither `Powered` nor `TogglePower`. A defense of this kind therefore refuses to charge while its house is short even if it draws no power at all, and an uncharged electric weapon reports that it needs to rearm.

### Fields, fences and lights

- A [`CloakGenerator=yes`](/keys/cloakgenerator/) structure collapses its field one ring per frame when it stops being operational and regrows it the same way afterwards. A generator left at `Powered=no` is exempt and keeps its field through any shortfall.
- [Laser fences](/systems/laser-fences/) are re-evaluated on every change to the balance, and a run energizes only while the posts at both ends are operational, so low power drops the whole run.
- A spotlight is neither drawn nor able to notice an intruder unless its structure is operational.
- At full power, powered animations are enabled and powered lights started for every structure the house owns. Below full power the matching shutdown runs only for a type that is `Powered=yes` with drain and `TogglePower=yes`.

:::caution[A sensor array does not go dark with the rest of the base]
Nothing lifts sensor coverage on a power change; a [`SensorArray=yes`](/keys/sensorarray/) structure gives its cells up only when it is taken off the map. What a shortfall costs it is the refresh, since the re-marking that follows a cloak field completing or another array shutting down skips an array that is not operational. Its cloak-generator counterpart, which shares [`CloakRadiusInCells`](/keys/cloakradiusincells/), collapses immediately.
:::

:::caution[A TogglePower=no defense is silenced but stays lit]
The animation shutdown and the out-of-service test both spare a `TogglePower=no` structure, while the weapon test does not. Such a defense holds fire through a shortfall with its idle animations and lights still running, which reads on screen as a working turret.
:::

### Player feedback

The low power announcement is made while all of these hold:

- the house is the local player's;
- its own announcement timer has expired;
- its power fraction is below 1;
- it has at least one structure of a [`BuildConst`](/keys/buildconst/) type on the map, switched on or not.

The announcement speaks the warning, posts the on-screen text for [`MessageDelay`](/keys/messagedelay/) minutes, and re-arms the timer with [`SpeakDelay`](/keys/speakdelay/) minutes.

The sidebar power bar is measured differently from the tally that runs the game. Its height comes from the type-level output and drain of every structure the player owns, leaving out the plugs, so a switched-off or half-destroyed structure still counts at its full rated figure. Only the split into colored bands is taken from the real tally. The bar can therefore look healthy while the base is running short of power.

## Scripting

The [Power Low](/mapping/events/tevent-low-power/) trigger event holds while the named house's fraction is below 1. The [Turn off building](/mapping/actions/taction-turn-off-attached/) and [Turn on building](/mapping/actions/taction-turn-on-attached/) actions reach the same switch the power cursor does, for every structure tagged with the trigger that is on the map. The first takes only the structures currently on and the second only those currently off, and a stunned structure refuses to come back on.

## Parsed settings without effect

The production ladder above is fixed in the engine. [`WorstLowPowerBuildRateCoefficient`](/keys/worstlowpowerbuildratecoefficient/) and [`BestLowPowerBuildRateCoefficient`](/keys/bestlowpowerbuildratecoefficient/) in `[General]` are read into the rules and nothing reads them. The gentler hard-coded step is 0.75 (the figure `BestLowPowerBuildRateCoefficient` already defaults to), which is why moving that setting appears to do nothing.
