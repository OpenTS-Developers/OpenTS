---
title: Warheads and damage
summary: "Gathers the objects one blast reaches and turns the warhead's figures into the strength each of them loses."
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

Damage reaches an object in two stages. A **blast** starts from one point on the map and gathers what stands near it. It holds a raw damage figure, the object to credit with whatever it kills, and a warhead. A shell landing raises one, as do a Tiberium field chain-reacting, an exploding barrel, a lightning bolt from an ion storm, and the charge an engineer leaves on a bridge. The **conversion** then runs separately for each object the blast gathered, turning that one raw figure into the strength that particular object loses. It also runs for damage that never came from a blast at all, such as [the tick a base takes while it is short of power](/systems/power/#the-structure-damage-tick).

This page owns both stages. [Firing geometry and beam weapons](/systems/firing-geometry/) settles the raw figure a shot leaves with. [Projectile flight and impact](/systems/projectile-flight/) settles where the blast is placed and how many blasts one shot delivers. What a destroyed object leaves behind belongs to [destruction and debris](/systems/destruction-and-debris/).

## Warheads in brief

A **WarheadType** is a rules section describing what damage does once it has arrived. The rules [`[Warheads]` list](/formats/rules-registries/) registers one, and a weapon's [`Warhead=`](/keys/warhead/#scope-weapontype) names the one its shots use. That page covers what a name nothing declares produces, and what happens to a weapon with no warhead at all.

The division of labor is fixed. The weapon holds how much damage a shot deals and the projectile holds how it travels. The warhead holds what that figure is worth against each kind of target and how quickly it thins with distance. It also holds every effect the blast has on the ground it lands on.

```ini title="rules.ini"
[MyShellWH]                    ; a WarheadType registered in [Warheads]
Verses=100%,80%,60%,40%,20%    ; against none, wood, light, heavy and concrete armor
Spread=5                       ; how slowly the damage thins with distance
ProneDamage=50%                ; what reaches an infantryman lying down
AnimList=MYBANG16,MYBANG24     ; example AnimTypes
```

`Verses` is a list of five entries in a fixed order, one for each [armor class](/reference/enums/armor/) an object type can have. The warhead above is therefore worth its whole figure against an unarmored target and a fifth of it against a concrete one.

Three warhead settings replace all of this rather than shaping it. A shot whose warhead is [`EMEffect=yes`](/keys/emeffect/) spends its figure as [a pulse](/systems/emp-pulse/). One whose warhead is [`Webby=yes`](/keys/webby/) spends it as a web, so neither reaches the two stages below. A weapon whose warhead sets a [`LimpetFactor`](/keys/limpetfactor/) of `1` or more fires no shot at all when its target is an object rather than a cell.

## What a blast reaches

The list of candidates is built first and in full, and only once it is complete is anything damaged. Two sweeps fill it, and a candidate gathered by either goes on to the same tests.

### The nine cells

The first sweep is by cell. The blast's own cell and the eight around it are walked in turn. Everything standing in each of them is added: a structure, a vehicle, an infantryman, a landed aircraft, a tree. Damage never spills further than one cell this way, whatever `Spread` says.

Two candidates are left out of this sweep:

- the object credited with the blast, so a shot never damages its own firer through this route;
- a vehicle whose type is named in [`HarvesterUnit`](/keys/harvesterunit/), while the harvester truce or a scenario's harvester-immunity option is in force.

A cell with an [`IsVeinholeMonster=yes`](/keys/isveinholemonster/) overlay also offers a [veinhole monster](/systems/veins/#destruction), which that page covers.

Which list of occupants each cell offers is settled once, from the blast's own cell alone. A lepton is the engine's internal distance unit, at 256 to the cell. Where that cell lies under a bridge and the blast stands more than 208 leptons above the ground there, all nine cells are read for what stands on their bridge decks. Otherwise all nine are read for what stands on the ground. A blast on a bridge therefore misses everything on the ground in the neighboring cells, and one under a bridge misses everything up on the deck.

### The airborne sweep

Whenever the blast stands above the ground at the middle of its own cell, a second sweep runs in addition to the first, and it is not restricted to cells at all. Three whole lists are walked: every aircraft in the game, every infantryman whose type is [`JumpJet=yes`](/keys/jumpjet/), and every vehicle whose type is [`Jellyfish=yes`](/keys/jellyfish/). Each one found is added under **all of:**

- it is placed on the map;
- it has strength left;
- it lies within one cell of the blast, measured in three dimensions, so height counts against the same allowance as ground distance.

This is the only route by which a blast reaches anything that is flying. An aircraft in flight and a jumpjet soldier in the air are both taken out of the cell they were standing in. Neither is entered in any cell's list of occupants, so the nine-cell sweep cannot see them however close the blast lands. A shell that goes off one lepton above the ground opens this sweep exactly as an airburst overhead does. A shot that lands flat on the ground opens it not at all.

Three things about the sweep are worth reading off its conditions:

- Nothing here asks whether the candidate is actually airborne. An aircraft sitting on the ground and a jumpjet soldier standing on its feet are both walked in by this sweep as well as by the cell sweep, and the duplicate is discarded.
- The object credited with the blast is not excluded here as it is from the cell sweep, so an aircraft can be caught by a blast it is itself responsible for.
- Every cell of [a wide-area blast](#the-wide-area-blast) is placed exactly at that cell's ground level, so none of them opens this sweep. A wide blast reaches nothing that is off the ground, however large its radius.

### Which candidates are damaged

Everything gathered is then tested, in the order the engine asks. A candidate is damaged under **all of:**

- it is still in play, which a candidate destroyed earlier in the same blast is not;
- it is not a structure whose type is [`InvisibleInGame=yes`](/keys/invisibleingame/);
- its strength is above zero;
- it is placed on the map and not in [limbo](/glossary/#limbo);
- it lies within one and a half cells of the blast, at [the distance the blast measures it at](#how-distance-thins-the-damage);
- **any of:**
  - the warhead is not the one [`IonStormWarhead`](/keys/ionstormwarhead/) names;
  - the candidate is neither an infantryman, a vehicle nor an aircraft;
  - it belongs to no team;
  - its team's type is not [`IonImmune=yes`](/keys/ionimmune/).

## What the target loses

The figure that arrives is the blast's raw damage, and it is the same figure for every candidate. Each one then works it down on its own account, so two objects standing in one explosion can lose very different amounts. The steps run in this order.

1. **A prone soldier.** An infantryman lying down keeps only the warhead's [`ProneDamage`](/keys/pronedamage/) fraction of the figure, and never less than one point.
2. **A web.** A [`Webby=yes`](/keys/webby/) warhead sets the figure to nothing for any infantryman that is not web-immune, and entangles it instead.
3. **The house divisors.** The figure is divided by the house's armor divisor and again by the object's own, which is `1` until an armor crate changes it. The house divisor is the product of [the country's](/keys/armor/#scope-housetype) figure and [the difficulty's](/keys/armor/#scope-difficulty-settings), which [difficulty settings](/systems/difficulty/#how-the-figures-are-combined) sets out.
4. **The veteran bonus.** An object holding the `STRONGER` [ability](/systems/veterancy/#abilities) divides again by [`VeteranArmor`](/keys/veteranarmor/) plus one.
5. **The floor of one.** Whatever steps 3 and 4 left is raised back to one point, so no divisor can take a hit to nothing.
6. **Type immunity.** [`TypeImmune=yes`](/keys/typeimmune/) ends the sequence outright when the attacker is the same type and belongs to the same house.
7. **Object immunity.** [`Immune=yes`](/keys/immune/#scope-aircrafttype) ends it as well, and so does a target already at zero strength.
8. **The armor table.** The figure is multiplied by the warhead's [`Verses`](/keys/verses/) entry for the target's [`Armor=`](/keys/armor/#scope-aircrafttype) class and truncated to a whole number.
9. **Distance.** The result is divided by the number of distance steps between the blast and the target, as the next section derives.
10. **The floor.** Below four distance steps, the result is raised to [`MinDamage`](/keys/mindamage/).
11. **The ceiling.** The result is capped at [`MaxDamage`](/keys/maxdamage/), once per object rather than once per blast.
12. **Applied.** What is left is taken off the target's strength, cut back first to the strength that was there. A killing blow is recorded at what the target still had rather than at what was aimed at it.

Damage the engine marks as forced skips almost all of it. Steps 1 and 3 through 6 are refused for a forced hit, step 7 is bypassed, and steps 8 through 11 are skipped entirely. A forced hit therefore lands at exactly its written figure. Only the web of step 2 and the application of step 12 still run. Two other things end the sequence at step 8: a blast with no warhead at all, and a scenario running with [`Inert=yes`](/keys/inert/). Either one reduces the figure to nothing there.

:::caution[`Verses=0%` is not immunity]
A product that truncates to zero at step 8 is replaced by one point rather than left at nothing. No percentage can therefore take a positive figure below a single point at that step. A warhead written at `0%` against an armor class still costs a target of that class the `MinDamage` figure close in. It stops costing it anything only once distance has thinned that one point away. The same replacement lifts any figure the multiply would have rounded off, which is why a small raw figure is barely reduced by the table at all. At a raw figure of `1`, every percentage below `200%` produces the same single point. `Immune=yes` is the setting that refuses damage outright.
:::

### How distance thins the damage

The distance a target is measured at is not simply how far it stands from the point of impact. Three adjustments come first.

- A height difference of less than one terrain level (104 leptons) is discarded. A target perched a fraction of a level above or below the blast is measured as though it stood level with it.
- An aircraft is measured at half its true distance. That widens the reach that finds it and softens the thinning that follows, so the same shot landing beside an aircraft and beside a vehicle hurts the aircraft more.
- A structure standing in the blast's own cell is measured at zero (a direct hit), however far its center lies from the impact. This holds only while nothing else stands in that cell. Structures are added to the end of a cell's list of occupants while this test reads the front of the list, so an infantryman sharing the cell displaces the structure from it. The structure is then measured from its own center like anything else.

That distance in leptons is then turned into a number of steps. It is divided by three times the warhead's [`Spread`](/keys/spread/#scope-warheadtype), and the result is held between `0` and `16`. The damage is divided by the number of steps, and a count of zero divides by nothing. Both divisions discard their remainder, which is what turns a smooth falloff into the handful of thresholds below. Every one of them is a distance in leptons and every one scales with `Spread`.

| | Distance |
| --- | --- |
| Full damage out to | 6 × `Spread` − 1 |
| Halved from | 6 × `Spread` |
| Down to a sixteenth from | 48 × `Spread` |
| `MinDamage` floor applies below | 12 × `Spread` |

At the default `Spread=1` that puts the halving 6 leptons out (a fortieth of a cell), so anything but a direct hit is thinned hard. It also puts the sixteenth step at 48 leptons, well inside the blast's one-and-a-half-cell reach. From `Spread=8` upward the sixteenth step lies beyond that reach entirely and is never used.

:::caution[`Spread=0` thins faster than `Spread=1`, and a negative figure thins not at all]
A `Spread` of zero does not switch the thinning off. It takes a separate branch that divides the distance by two rather than by three, so the damage thins half again as fast as the default does. A negative figure switches it off completely: the step count comes out negative, is clamped to zero, and every target inside the blast's reach takes the full armor-scaled figure with the `MinDamage` floor applied on top.
:::

## Healing

A weapon whose [`Damage=`](/keys/damage/#scope-weapontype) is negative mends rather than hurts, and the figure takes a different route through everything above. None of the reductions touch it. The prone fraction, the house divisors, the veteran bonus, type immunity, the armor table, the distance thinning, the floor and the ceiling are all refused or returned before. Two conditions decide whether it lands:

- the target lies within 8 leptons of the blast (a thirty-second of a cell), so a healing shot mends what it strikes and nothing standing beside it;
- the target is not `Immune=yes` and has strength left, since nothing revives an object already at zero.

What lands is the whole figure, added to strength and clamped at the type's maximum. The hit is reported as no damage at all, so nothing springs from it and the target does not turn on the healer.

Clearing a [limpet](/keys/limpetfactor/) mark is the one consequence that does not wait for the 8-lepton test. It runs for every object the blast reached (everything inside the one-and-a-half-cell test) and ahead of the immunity check. A healing blast therefore strips the marks off everything around it and mends only what it landed on.

## What one blast does besides damage

Everything above concerns objects. A blast also works on the ground it lands on, and those effects run after the damage has been handed out, in this order. Those that weigh a figure at all weigh the blast's raw damage rather than what any object ended up losing. Armor and distance therefore never move a blast across one of their thresholds.

1. [`Rocker=yes`](/keys/rocker/) tips the vehicles in the seven-by-seven block of cells around the blast, once the blast's raw figure is above 30. No setting moves that threshold.
2. A [`ChainReaction=yes`](/keys/chainreaction/) overlay in the blast's own cell is set off, and that cell's Tiberium is reduced by a tenth of the figure. A Tiberium overlay also requires a [`Tiberium=yes`](/keys/tiberium/#scope-warheadtype) warhead, and several of the blasts the engine raises for itself ask for no chain reaction at all.
3. A wall overlay in that cell is reduced by the figure, for a [`Wall=yes`](/keys/wall/#scope-warheadtype) warhead or a [`Wood=yes`](/keys/wood/) one standing over a wood-armored wall.
4. A bridge span at that cell is rolled for against [`BridgeStrength`](/keys/bridgestrength/), with [`DestroyableBridges=yes`](/keys/destroyablebridges/) and a `Wall=yes` warhead.
5. An [`Explodes=yes`](/keys/explodes/#scope-overlaytype) overlay in that cell is cleared and goes off for [`AmmoCrateDamage`](/keys/ammocratedamage/).
6. The ground is cratered on a [`Deform`](/keys/deform/) roll, made only above [`DeformThreshhold`](/keys/deformthreshhold/).
7. A destroyable cliff at that cell is rolled for against [`CollapseChance`](/keys/collapsechance/).
8. The warhead's [`Particle`](/keys/particle/) system releases one particle.
9. A `Wall=yes` or [`Fire=yes`](/keys/fire/) warhead cracks the ice beneath the blast.

The explosion animation and the lighting flash are not among them. [`AnimList`](/keys/animlist/), the [`SplashList`](/keys/splashlist/) substitution [`Conventional=yes`](/keys/conventional/) makes over water, and the [`Bright=yes`](/keys/bright/#scope-warheadtype) flash are all read by whatever raised the blast rather than by the blast itself. Which of them appear therefore depends on the path that produced the explosion as well as on the warhead.

Three more warhead settings act on the victim rather than on the blast, and each is owned where its consequence is. [`InfDeath`](/keys/infdeath/) picks the death a killed infantryman performs. [`Sparky=yes`](/keys/sparky/) chooses the flames a structure shows as it drops a damage level and sets a struck tree alight. [`Veinhole=yes`](/keys/veinhole/) decides who the victim blames for damage that names no attacker. [Target selection](/systems/target-selection/#retaliation) sets out that route from a hit to a shot fired back along with every other one.

## The wide-area blast

A second routine exists for blasts far larger than one and a half cells. It walks a square block of cells around its center and puts an ordinary blast into each one. Each cell's share is scaled by how far that cell lies from the middle. [`ExpSpread`](/keys/expspread/) owns that scaling, which does not work the way the falloff above does.

No shot landing ever reaches it. The only route to it from a detonating projectile is a fallback meant to cover a nuclear blast whose explosion animation could not be built. [`AtomDamage`](/keys/atomdamage/) records why that fallback is never taken. Two paths do reach the routine, and both are deaths rather than impacts. One is the collateral blast an [`Explodes=yes`](/keys/explodes/#scope-aircrafttype) object makes as it dies, and the other is the extra blast a loaded harvester adds under [`TiberiumExplosive=yes`](/keys/tiberiumexplosive/#scope-global-rules).
