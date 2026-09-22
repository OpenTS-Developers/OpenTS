---
title: Warheads and damage
summary: "How a blast chooses the objects it damages, how much strength each one loses, and what else the blast does to the ground."
category: weapons-projectiles
keys:
  - AmmoCrateDamage
  - AnimList
  - Armor
  - AtomDamage
  - BridgeStrength
  - Bright
  - ChainReaction
  - CollapseChance
  - Conventional
  - Damage
  - Deform
  - DeformThreshhold
  - DestroyableBridges
  - EMEffect
  - ExpSpread
  - Explodes
  - Fire
  - HarvesterUnit
  - Immune
  - Inert
  - InfDeath
  - InvisibleInGame
  - IonImmune
  - IonStormWarhead
  - IsVeinholeMonster
  - Jellyfish
  - JumpJet
  - LimpetFactor
  - MaxDamage
  - MinDamage
  - Particle
  - ProneDamage
  - Rocker
  - Sparky
  - SplashList
  - Spread
  - Tiberium
  - TiberiumExplosive
  - TypeImmune
  - Veinhole
  - Verses
  - VeteranArmor
  - Wall
  - Warhead
  - Webby
  - Wood
related:
  - type: system
    id: projectile-flight
  - type: system
    id: firing-geometry
  - type: system
    id: destruction-and-debris
  - type: system
    id: target-selection
  - type: system
    id: veterancy
  - type: system
    id: difficulty
  - type: system
    id: emp-pulse
  - type: enum
    id: ArmorType
---

Damage reaches an object in two stages. First, a **blast** goes off at one point on the map and collects the objects near it. A blast carries a raw damage figure, a warhead, and the object credited with anything it kills. A landing shell raises a blast. So do a chain-reacting Tiberium field, an exploding barrel, a lightning bolt from an ion storm, and the [C4](/keys/c4/) charge an infantryman sets on a bridge. Second, each object the blast collected converts the raw figure into the strength it loses. This conversion also runs for damage that comes from no blast, such as [the damage a base takes while it is short of power](/systems/power/#the-structure-damage-tick).

This page covers both stages. [Firing geometry and beam weapons](/systems/firing-geometry/) sets the raw figure a shot carries. [Projectile flight and impact](/systems/projectile-flight/) sets where the blast goes off and how many blasts one shot raises. [Destruction and debris](/systems/destruction-and-debris/) covers what a destroyed object leaves behind.

## Warheads in brief

A **WarheadType** is a rules section that describes what damage does once it arrives. The rules [`[Warheads]` list](/formats/rules-registries/) registers each one, and a weapon's [`Warhead=`](/keys/warhead/#scope-weapontype) names the one its shots use. The `Warhead=` page covers a name that no section declares and a weapon with no warhead.

The weapon sets how much damage a shot deals, and the projectile sets how the shot travels. The warhead sets how much of that damage each armor class takes and how quickly the damage thins with distance. It also sets every effect the blast has on the ground.

```ini title="rules.ini"
[MyShellWH]                    ; a WarheadType registered in [Warheads]
Verses=100%,80%,60%,40%,20%    ; against none, wood, light, heavy and concrete armor
Spread=5                       ; how slowly the damage thins with distance
ProneDamage=50%                ; what reaches an infantryman lying down
AnimList=MYBANG16,MYBANG24     ; example AnimTypes
```

`Verses` lists five percentages in a fixed order, one for each [armor class](/reference/enums/armor/) an object type can have. The warhead above deals its whole figure to an unarmored target and a fifth of it to a concrete one. The values are examples.

Three warhead settings replace blast damage entirely:

- A shot whose warhead is [`EMEffect=yes`](/keys/emeffect/) raises [a pulse](/systems/emp-pulse/) instead of a blast.
- A shot whose warhead is [`Webby=yes`](/keys/webby/) spreads a web over the surrounding cells instead of raising a blast.
- A weapon whose warhead sets [`LimpetFactor`](/keys/limpetfactor/) to `1` or more never fires a shot at a vehicle, infantryman, aircraft or structure. The `LimpetFactor` page covers what it does instead.

## What a blast reaches

A blast first collects its full list of candidates, and only then damages any of them. Two sweeps fill the list. The cell sweep always runs, and the airborne sweep runs only when the blast is above the ground. Every candidate from either sweep then faces the same tests.

### The nine cells

The cell sweep collects everything standing in the blast's cell and the eight cells around it: structures, vehicles, infantry, landed aircraft and trees. It never reaches further than one cell from the blast's cell, whatever `Spread` says.

The cell sweep leaves out two kinds of candidate:

- the object credited with the blast, so a shot cannot damage its firer through this sweep;
- a vehicle whose type is listed in [`HarvesterUnit`](/keys/harvesterunit/), while the harvester truce or the scenario's [`HarvesterImmune`](/keys/harvesterimmune/) flag is on.

A blast centered on a [veinhole monster's](/systems/veins/#destruction) cell also collects the monster, provided its overlay sets [`IsVeinholeMonster=yes`](/keys/isveinholemonster/).

The blast's cell decides whether the sweep collects objects on bridge decks or objects on the ground, and that choice applies to all nine cells. A [lepton](/glossary/#lepton) is the engine's distance unit, and a cell is 256 leptons across. When the blast's cell lies under a bridge and the blast is more than 208 leptons above the ground there, the sweep collects what stands on the bridge decks. Otherwise it collects what stands on the ground. A blast on a bridge therefore misses everything on the ground in the neighboring cells, and a blast under a bridge misses everything on the deck.

### The airborne sweep

When the blast is above the ground level at the center of its cell, a second sweep runs. It checks three whole lists of objects, not cells: every aircraft, every infantryman whose type is [`JumpJet=yes`](/keys/jumpjet/), and every vehicle whose type is [`Jellyfish=yes`](/keys/jellyfish/). It collects each one that meets **all of:**

- it is placed on the map;
- it has strength left;
- it is less than one cell from the blast, measured in three dimensions, so height counts the same as ground distance.

This sweep is the only way a blast reaches a flying object. An aircraft in flight and a jumpjet infantryman in the air are not listed in any cell, so the cell sweep cannot find them however close the blast lands. A shell that explodes one lepton above the ground runs this sweep just as an airburst overhead does. A shot that explodes at ground level does not run it.

The sweep's conditions have three consequences:

- The sweep does not check whether a candidate is actually in the air. A landed aircraft or a jumpjet infantryman standing on the ground can be found by both sweeps, but it is collected only once.
- The sweep does not leave out the object credited with the blast, so an aircraft can be damaged by a blast credited to it.
- Each cell of [a wide-area blast](#the-wide-area-blast) goes off exactly at ground level, so a wide-area blast never runs this sweep and never reaches a flying object.

### Which candidates are damaged

Each candidate is then tested in the order below. It is damaged only if it meets **all of:**

- it is still in play;
- it is not a structure whose type is [`InvisibleInGame=yes`](/keys/invisibleingame/);
- its strength is above zero;
- it is placed on the map and not in [limbo](/glossary/#limbo);
- it is less than one and a half cells from the blast, measured as [the distance section](#how-distance-thins-the-damage) describes;
- **any of:**
  - the warhead is not the one [`IonStormWarhead`](/keys/ionstormwarhead/) names;
  - the candidate is not an infantryman, vehicle or aircraft;
  - the candidate belongs to no team;
  - the candidate's team type is not [`IonImmune=yes`](/keys/ionimmune/).

## What the target loses

Every candidate receives the same raw figure and then reduces it separately, so two objects in the same explosion can lose very different amounts. The steps run in this order:

1. **Prone infantry.** An infantryman lying down keeps only the warhead's [`ProneDamage`](/keys/pronedamage/) fraction of the figure, rounded down but never below one point.
2. **Web.** A [`Webby=yes`](/keys/webby/) warhead entangles any infantryman whose type is not [`IsWebImmune=yes`](/keys/iswebimmune/). The figure becomes zero, so the hit does nothing else.
3. **Armor multipliers.** The figure is divided by the owning house's armor multiplier and by the object's crate armor multiplier, then rounded down. The crate multiplier stays `1` until an armor crate raises it. In a campaign, the house multiplier is the difficulty's [`Armor=`](/keys/armor/#scope-difficulty-settings). In any other game, it is that figure multiplied by the country's [`Armor=`](/keys/armor/#scope-housetype). [Difficulty settings](/systems/difficulty/#how-the-figures-are-combined) shows how the two combine.
4. **Veteran armor.** An object with the `STRONGER` [ability](/systems/veterancy/#abilities) divides the figure again by one plus [`VeteranArmor`](/keys/veteranarmor/), rounded down.
5. **Minimum of one.** If steps 3 and 4 left less than one point, the figure becomes one point. Armor multipliers cannot reduce a hit to nothing.
6. **Type immunity.** An object whose type is [`TypeImmune=yes`](/keys/typeimmune/) takes no damage from a credited attacker of the same type owned by the same house.
7. **Object immunity.** An object whose type is [`Immune=yes`](/keys/immune/#scope-aircrafttype) takes no damage. Neither does an object already at zero strength.
8. **Armor table.** The figure is multiplied by the warhead's [`Verses`](/keys/verses/) entry for the target's [`Armor=`](/keys/armor/#scope-aircrafttype) class and rounded down. A result of zero becomes one point.
9. **Distance.** The result is divided by the number of distance steps between the blast and the target, as [the next section](#how-distance-thins-the-damage) explains.
10. **`MinDamage` floor.** When the target is fewer than four distance steps away, the result is raised to at least [`MinDamage`](/keys/mindamage/).
11. **`MaxDamage` ceiling.** The result is capped at [`MaxDamage`](/keys/maxdamage/). The cap applies to each object separately, not to the blast as a whole.
12. **Strength lost.** The result is taken off the target's strength, but never more than the strength it had. A killing blow therefore counts only the strength the target had left.

Steps 3 to 6 apply only to vehicles, infantry, aircraft and structures, so a tree or a veinhole monster skips them.

A tree takes no damage at all unless the warhead is [`Wood=yes`](/keys/wood/).

A blast never deals forced damage. The engine deals forced damage directly, for example when a C4 charge destroys a structure. Forced damage skips step 1 and steps 3 to 11, so of the numbered steps only the web of step 2 and step 12 apply. It still does nothing to an object already at zero strength. Refusals that belong to a particular kind of object also still apply, such as a tree's need for a `Wood=yes` warhead and a harvester's protection under the harvester truce.

Unforced damage with no warhead, or any unforced damage in a scenario with [`Inert=yes`](/keys/inert/), does nothing, because step 8 gives zero. An ordinary blast in either case damages nothing and has none of the ground effects below. A [wide-area blast](#the-wide-area-blast) in an `Inert=yes` scenario can still crater the ground at its center.

:::caution[`Verses=0%` is not immunity]
At step 8, a result that rounds down to zero becomes one point, so no `Verses` percentage can take a positive figure below one point. A warhead with `0%` against an armor class still deals `MinDamage` to targets of that class near the blast. It stops harming them only from the fourth distance step outward, where the single point divided by the step count rounds down to zero. Small raw figures are barely changed by the table for the same reason: at a raw figure of `1`, every percentage below `200%` gives one point. To make an object type take no damage, set `Immune=yes` on it.
:::

### How distance thins the damage

The distance used for a target differs from its plain distance to the point of impact in three ways:

- A height difference of less than one terrain level (104 leptons) is ignored. A target slightly above or below the blast counts as level with it.
- An aircraft counts at half its true distance, so it takes more damage than a vehicle at the same distance. The halved distance is also used for the one-and-a-half-cell reach test, but the airborne sweep's one-cell limit uses the true distance.
- A structure in the blast's cell counts as a direct hit, at distance zero, however far its center is from the impact. This applies only while no other object stands in that cell. If an infantryman shares the cell, for example, the structure's distance is measured normally.

The distance in leptons is then converted to a number of distance steps. It is divided by three times the warhead's [`Spread`](/keys/spread/#scope-warheadtype), rounded down, and held between `0` and `16`. The damage is then divided by the number of steps and rounded down. At `0` steps the damage is not divided.

Because both divisions round down, damage falls at the fixed distances below instead of smoothly. Each distance is in leptons and scales with `Spread`.

| | Distance |
| --- | --- |
| Full damage out to | 6 × `Spread` − 1 |
| Halved from | 6 × `Spread` |
| Down to a sixteenth from | 48 × `Spread` |
| `MinDamage` floor applies below | 12 × `Spread` |

At the default `Spread=1`, damage halves 6 leptons from the impact, about a fortieth of a cell, so any target not hit directly loses much of the damage. Damage drops to a sixteenth at 48 leptons, well within the blast's one-and-a-half-cell reach. At `Spread=8` or more, the sixteenth step lies beyond that reach and never applies.

:::caution[`Spread=0` does not turn off thinning]
At `Spread=0` the distance is divided by two instead of by three times `Spread`, so damage thins one and a half times as fast as at `Spread=1`. A negative `Spread` does turn thinning off. Every target within the blast's reach takes the full armor-adjusted figure, raised to `MinDamage` if it is lower.
:::

## Healing

A weapon with a negative [`Damage=`](/keys/damage/#scope-weapontype) heals. A healing figure skips every reduction above: the prone fraction, the armor multipliers, veteran armor, type immunity, the armor table, distance thinning, `MinDamage` and `MaxDamage`. The target regains the whole figure, up to its type's maximum strength, if it meets **all of:**

- it is less than 8 leptons from the blast, a thirty-second of a cell, so a healing shot repairs only what it hits and nothing beside it;
- it is not `Immune=yes`;
- it still has strength left, so nothing destroyed is revived.

No trigger event springs from a heal, and the healed object does not fire back at the healer.

Healing also clears every [limpet](/keys/limpetfactor/) mark from a vehicle, infantryman, aircraft or structure. This happens to every such object the blast reached within one and a half cells, before the 8-lepton and immunity tests. A healing blast therefore strips limpet marks from everything around it but repairs only what it hits.

## What one blast does besides damage

After it damages objects, a blast acts on the ground in the order below. Every threshold below is compared with the blast's raw figure, not with the damage any object took. Armor and distance therefore never decide whether those effects happen.

1. [`Rocker=yes`](/keys/rocker/) rocks every object drawn as a voxel in the seven-by-seven block of cells around the blast, when the raw figure is above 30. No setting changes that threshold.
2. A Tiberium overlay in the blast's cell that sets [`ChainReaction=yes`](/keys/chainreaction/) may chain-react when the warhead is [`Tiberium=yes`](/keys/tiberium/#scope-warheadtype). The same hit takes a tenth of the raw figure off that cell's Tiberium. A chain-reactive overlay that is not Tiberium does not react on the flag alone, as the `ChainReaction` page explains. Several blasts the engine raises itself, including the blasts of a chain reaction, skip this step.
3. A wall overlay in the blast's cell may drop one damage level, and one blast never takes it down more than one. The warhead must be [`Wall=yes`](/keys/wall/#scope-warheadtype), or [`Wood=yes`](/keys/wood/) against a wood-armored wall. The wall always drops when the raw figure is at least the overlay's [`Strength`](/keys/strength/#scope-overlaytype). Below that, the chance is about the raw figure divided by that `Strength`.
4. A bridge span at the blast's cell may be damaged, when [`DestroyableBridges=yes`](/keys/destroyablebridges/) and the warhead is `Wall=yes`. [`BridgeStrength`](/keys/bridgestrength/) sets the chance.
5. An [`Explodes=yes`](/keys/explodes/#scope-overlaytype) overlay in the blast's cell is removed and explodes for [`AmmoCrateDamage`](/keys/ammocratedamage/), whatever the warhead.
6. The ground may be cratered when the raw figure is above [`DeformThreshhold`](/keys/deformthreshhold/). [`Deform`](/keys/deform/) sets the chance.
7. A destroyable cliff at the blast's cell may collapse, with a chance of [`CollapseChance`](/keys/collapsechance/), whatever the warhead.
8. The warhead's [`Particle`](/keys/particle/) system releases one particle at the blast.
9. A `Wall=yes` or [`Fire=yes`](/keys/fire/) warhead cracks the ice beneath the blast, unless the blast is up on a bridge.

The explosion animation and the lighting flash are not blast effects. [`AnimList`](/keys/animlist/), the [`SplashList`](/keys/splashlist/) animation that [`Conventional=yes`](/keys/conventional/) uses over water, and the [`Bright=yes`](/keys/bright/#scope-warheadtype) flash are chosen by whatever raised the blast. Whether they appear therefore depends on what caused the explosion as well as on the warhead.

Three more warhead settings act on the damaged object. Each is described on the page that covers its effect:

- [`InfDeath`](/keys/infdeath/) picks the death animation a killed infantryman plays.
- [`Sparky=yes`](/keys/sparky/) chooses the flames a structure shows as it drops a damage level, and sets a struck tree alight.
- [`Veinhole=yes`](/keys/veinhole/) gives a damaged object something to fire back at when no attacker is credited. [Target selection](/systems/target-selection/#retaliation) covers when a damaged object fires back.

## The wide-area blast

A wide-area blast raises an ordinary blast in every cell of a square block. The center cell's blast carries the full raw figure. Every other cell's blast carries the figure scaled by that cell's distance from the center, so the figure rises toward the rim instead of falling. [`ExpSpread`](/keys/expspread/) gives the size of the block and the scale.

No landing shot raises a wide-area blast. Two deaths do:

- the collateral blast of a dying object whose type is [`Explodes=yes`](/keys/explodes/#scope-aircrafttype) or that has the `EXPLODES` [ability](/systems/veterancy/#abilities);
- the extra blast a destroyed harvester adds for the Tiberium it carries, under [`TiberiumExplosive=yes`](/keys/tiberiumexplosive/#scope-global-rules), unless the harvester truce or the scenario's `HarvesterImmune` flag is on.

A nuclear detonation has a fallback route to a wide-area blast for when its explosion animation cannot be created. [`AtomDamage`](/keys/atomdamage/) explains why that route is never taken.
