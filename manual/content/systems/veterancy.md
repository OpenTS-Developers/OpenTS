---
title: Veterancy and promotion
summary: "How experience, promotion, abilities, and elite weapons work."
category: combat-targeting
keys:
  - Armory
  - CrateRadius
  - Elite
  - EliteAbilities
  - EliteFlashTimer
  - IRepairRate
  - InitialVeteran
  - Trainable
  - UpgradeEliteSound
  - UpgradeVeteranSound
  - VeteranAbilities
  - VeteranArmor
  - VeteranCap
  - VeteranCombat
  - VeteranFlashTimer
  - VeteranLevel
  - VeteranROF
  - VeteranRatio
  - VeteranSight
  - VeteranSpeed
  - VoxUnitPromoted
related:
  - type: system
    id: drop-pods
  - type: format
    id: teamtypes
  - type: enum
    id: CrateType
---

## Ranks

Vehicles, infantry, aircraft, and buildings each have their own experience total. That total determines their rank:

| Rank | Experience |
| --- | --- |
| Below rookie | less than `0` |
| Rookie | `0` up to but not including `1` |
| Veteran | `1` up to but not including `2` |
| Elite | `2` and above |

Objects start at rookie with `0` experience unless another setting gives them a different rank. Veteran always starts at `1` and elite at `2`. Rules can change how quickly experience is earned, but these thresholds are fixed.

[`VeteranLevel=0`](/keys/veteranlevel/) gives an object `-0.25` experience, placing it below rookie. A map's [placed-object records](/formats/scenario-objects/) can also set experience directly, up to [`VeteranCap`](/keys/veterancap/). Below-rookie objects have no rank abilities, just like rookies. They differ in their insignia and in how crates and armories promote them, as described below.

## Earning experience

The object that deals the fatal damage earns experience if its type has [`Trainable=yes`](/keys/trainable/). Each kill adds `victim cost / (killer cost * VeteranRatio)` to its experience. The costs come from [`Cost`](/keys/cost/#scope-aircrafttype); [`Points`](/keys/points/) does not affect experience.

Starting from rookie, an object must destroy enemies worth [`VeteranRatio`](/keys/veteranratio/) times its own cost to reach veteran, and twice that to reach elite. A lower `VeteranRatio` means faster promotion. Cheaper types need fewer kills against the same enemies.

```ini title="rules.ini"
[General]
VeteranRatio=5
VeteranCap=2
```

This example allows promotion through combat up to elite rank. [`VeteranCap`](/keys/veterancap/) sets the experience limit. These are example values; the linked key pages give the defaults.

:::note[Cost multipliers apply to both sides]
Both costs use the defeated owner's [cost multipliers](/keys/cost/#what-a-house-pays). Applying the same multiplier to both costs leaves their ratio unchanged, except for whole-credit rounding. The attacker's own price multipliers are ignored.
:::

Buildings must explicitly set `Trainable=yes` to earn experience from kills. `Trainable` does not control rank benefits: an untrainable object still benefits from a rank given by an armory, a trigger, or another source that ignores this setting.

A building with a [`FreeUnit`](/keys/freeunit/) usually awards experience based on the building's full price, even after the free unit has left. If that unit costs more than the building, its price is used instead. See [structure costs](/keys/cost/#what-a-structure-gives-away) for the calculation.

Keep `VeteranRatio` and the killer's cost above `0`: the formula divides by both. With a positive victim cost, a zero divisor sets experience to `VeteranCap`.

A kill gives no experience if the defeated owner considers the killer an ally. Score, loss counts, and destruction triggers still update.

Destroying a transport also awards experience for each passenger killed inside it. Crushing an object counts as a kill for the vehicle that crushed it.

:::note[Capturing awards no experience]
Capturing an object gives score points equal to its cost, but no experience. Selling a building or losing an object by sinking gives neither score points nor experience to anyone.
:::

Experience does not decrease over time, and changing owners does not reset it.

### The experience ceiling

Every experience gain is limited to [`VeteranCap`](/keys/veterancap/). That covers kills, veterancy-crate steps, armory promotions, `InitialVeteran`, a reinforcement's `VeteranLevel`, the Drop Pods superweapon, the Make Elite trigger action, and experience from a map's placed-object records.

:::caution[Allowing elite rank]
Set `VeteranCap` to at least `2` to allow elite rank from any source. The default limit is `1`, so every promotion stops at veteran.
:::

:::caution[A low limit can demote an elite]
An object above the limit drops back to it on its next kill that earns experience. For example, with `VeteranCap=1`, a trainable elite drops to veteran and loses its elite weapon and [`EliteAbilities`](/keys/eliteabilities/).
:::

## Promotion without kills

The following sources can give an object a rank without kills. Every result stops at [`VeteranCap`](/keys/veterancap/); only the veterancy crate requires `Trainable=yes`. A map's placed-object records can also set experience directly.

| Source | Result |
| --- | --- |
| [Veterancy crate](/reference/enums/crate/) | Every `Trainable=yes` object on the ground within [`CrateRadius`](/keys/crateradius/) of the crate rises one rank, repeated as many times as the crate's `Data` figure in `[Powerups]` says, which [Crate powerups](/formats/powerups/) covers. |
| [`Armory=yes`](/keys/armory/) building | The infantry inside is promoted once the building's servicing counter runs out. |
| [Drop Pods superweapon](/systems/drop-pods/#drop-pods-superweapon) | Each delivered passenger is created elite. |
| TeamType [`VeteranLevel`](/keys/veteranlevel/) | Every member created for the team takes the rank the value selects. |
| [`InitialVeteran=yes`](/keys/initialveteran/) | The units and infantry drawn from the random starting selection of a skirmish or multiplayer match are created elite. |
| [Make Elite trigger action](/mapping/actions/taction-make-elite/) | Every object the trigger is attached to is promoted to elite. |

With the default limit of `1`, the rows that say elite leave the object veteran.

A crate promotes each affected object one step at a time: below rookie to rookie, rookie to veteran, then veteran to elite. Elite objects stay elite. The crate's `Data` value in `[Powerups]` sets the number of steps. Fractions round up, so `2.5` gives three steps and `2` gives two. A value of `0` or less gives no promotion.

The other sources set the rank directly. The armory's result depends on the infantry's current rank.

:::caution[A veterancy crate promotes every trainable object nearby]
Crates also promote enemy and neutral objects within `CrateRadius`, including buildings, if their types have `Trainable=yes`.
:::

:::caution[An armory skips the veteran rank]
Rookie and veteran infantry leave an armory as elite, and below-rookie infantry as veteran, unless [`VeteranCap`](/keys/veterancap/) is below `2`. With the default limit of `1`, every admission leaves the infantry veteran. Elite infantry do not get an enter cursor. Each admission uses one point of the building's [`Ammo`](/keys/ammo/), which an armory never replenishes.
:::

The training time is set by [`IRepairRate`](/keys/irepairrate/), which also controls hospital treatment time.

## What a rank changes

Promotion bonuses depend on the type's ability lists. A veteran with no abilities gains only a rank insignia. Elite objects can also use an elite weapon and cause other objects in their cell to scatter, as described below.

### Abilities

[`VeteranAbilities`](/keys/veteranabilities/) and [`EliteAbilities`](/keys/eliteabilities/) are comma-separated lists on each object type. Veterans use `VeteranAbilities`. Elites use both lists, keeping their veteran abilities after promotion. Rookie and below-rookie objects use neither list.

```ini title="rules.ini"
[MYINF] ; example InfantryType
VeteranAbilities=FIREPOWER,ROF
EliteAbilities=SELF_HEAL,FEARLESS
```

A veteran `MYINF` deals more damage and reloads faster. At elite rank it also heals itself and becomes immune to fear.

Ability names are case-insensitive. The table lists all eighteen abilities and their effects.

| Token | Effect on a qualifying object |
| --- | --- |
| `FASTER` | Movement speed is multiplied by [`VeteranSpeed`](/keys/veteranspeed/) plus one. Buildings have no speed to raise. |
| `STRONGER` | Incoming damage is divided by [`VeteranArmor`](/keys/veteranarmor/) plus one. |
| `FIREPOWER` | Weapon damage is multiplied by [`VeteranCombat`](/keys/veterancombat/) plus one. |
| `SCATTER` | The object scatters from incoming fire even when the owner's units would otherwise hold position. |
| `ROF` | Reload delay is divided by [`VeteranROF`](/keys/veteranrof/) plus one. |
| `SIGHT` | Sight range is multiplied by [`VeteranSight`](/keys/veteransight/) plus one. |
| `CLOAK` | The object cloaks and recloaks without [`Cloakable=yes`](/keys/cloakable/), and stays cloaked while immobilized. |
| `TIBERIUM_PROOF` | Infantry take no damage from standing in Tiberium, as with [`TiberiumProof=yes`](/keys/tiberiumproof/). |
| `VEIN_PROOF` | Veins do not damage the object, as with [`ImmuneToVeins=yes`](/keys/immunetoveins/). |
| `SELF_HEAL` | The object repairs itself while below its [self-healing ceiling](/systems/repair/#self-healing), as with [`SelfHealing=yes`](/keys/selfhealing/). |
| `EXPLODES` | Death produces the violent explosion of [`Explodes=yes`](/keys/explodes/). |
| `RADAR_INVISIBLE` | The object is kept off the local player's radar unless a sensor contact picks it up. |
| `SENSORS` | An adjacent enemy cloaked object shimmers, as with [`Sensors=yes`](/keys/sensors/). |
| `FEARLESS` | Infantry accumulate no fear, as with [`Fearless=yes`](/keys/fearless/). |
| `C4` | Infantry attack buildings by sabotage, as with [`C4=yes`](/keys/c4/). |
| `TIBERIUM_HEAL` | Standing in Tiberium repairs the object, as with [`TiberiumHeal=yes`](/keys/tiberiumheal/). |
| `GUARD_AREA` | An idle armed vehicle, and an idle human-owned infantry, takes the Guard Area mission instead of Guard. A member of a team is unaffected, and so is a computer-owned infantry, whose idle handling never reads the ability. |
| `CRUSHER` | A vehicle crushes crushable objects and overlays, as with [`Crusher=yes`](/keys/crusher/). |

Unknown or misspelled ability names are ignored without a warning.

:::caution[Write ability lists without spaces]
Spaces around individual names are not removed. For example, `VeteranAbilities=FIREPOWER, ROF` enables only `FIREPOWER`, because the space before `ROF` makes it an unknown name. Spaces at the start or end of the whole value are removed.
:::

Setting an ability list in a later rules file replaces the earlier list. Leaving the key out keeps the earlier list. An empty assignment is ignored, so it cannot clear an existing list.

:::caution[An elite object makes its whole cell scatter]
Incoming fire at a cell containing an elite object makes all occupants scatter, even those without `SCATTER`. This effect requires no ability.
:::

### Rank multipliers

Set the five bonuses below in `rules.ini` under `[General]`. Each applies only when the object has the matching ability:

| Setting | Applied as |
| --- | --- |
| [`VeteranCombat`](/keys/veterancombat/) | Damage is multiplied by the value plus one, after the house and object firepower biases. |
| [`VeteranSpeed`](/keys/veteranspeed/) | Speed is multiplied by the value plus one, after the house ground-speed bias. |
| [`VeteranSight`](/keys/veteransight/) | Sight range is multiplied by the value plus one. |
| [`VeteranArmor`](/keys/veteranarmor/) | Incoming damage is divided by the value plus one. |
| [`VeteranROF`](/keys/veteranrof/) | Reload delay is divided by the value plus one. |

`VeteranArmor=1` halves incoming damage, and `VeteranROF=1` halves reload delay. For the other three settings, `1` doubles the affected statistic. A value of `0` gives no bonus. All five default to `1`.

Sonic weapons and weapons using fire particles receive no firepower bonus. Sonic weapons also receive no reload bonus, as do weapons with an attached spark, fire, or railgun particle system. The reload bonus shortens the delay after a burst, not the gaps between its shots.

### The elite weapon

At elite rank, the weapon named by [`Elite`](/keys/elite/) replaces the primary weapon. Target selection, range, reload delay, and firing all use this weapon. The secondary weapon stays unchanged. If `Elite` is unset, the object keeps its primary weapon.

The elite weapon uses the primary weapon's art settings: [`PrimaryFireFLH`](/keys/primaryfireflh/), [`PBarrelLength`](/keys/pbarrellength/), and [`PBarrelThickness`](/keys/pbarrelthickness/). It fires from the same muzzle offset.

:::caution[Building upgrades take priority]
A building upgrade that supplies a weapon takes priority over the building's own weapon in that slot, including its elite weapon. Upgrades are structure types with [`PowersUpBuilding=`](/keys/powersupbuilding/) installed in the host's [upgrade slots](/keys/upgrades/).
:::

### When a promotion takes effect

Promotion bonuses apply the next time the object fires, takes damage, or moves. The game checks its current rank and abilities whenever it calculates those effects.

Extra sight reveals more terrain at once: a promotion makes the object reveal its surroundings again, so a `SIGHT` bonus uncovers the larger area immediately.

## Promotion feedback

The game announces a promotion with a sound, an EVA line, and a flash of the promoted object. It announces combat promotions, promotions from a veterancy crate, and promotions from an armory. The remaining sources in [Promotion without kills](#promotion-without-kills) set the rank without announcing it, and a rise from below rookie to rookie is silent.

Only the local player's houses produce this feedback. Another house's promotion still changes its rank and insignia, but adds no sound, line, or flash on your machine.

The rank picks the sound and the flash length: [`UpgradeVeteranSound`](/keys/upgradeveteransound/) and [`VeteranFlashTimer`](/keys/veteranflashtimer/) for a rise to veteran, and [`UpgradeEliteSound`](/keys/upgradeelitesound/) and [`EliteFlashTimer`](/keys/eliteflashtimer/) for a rise to elite, including one that skips the veteran rank. [`VoxUnitPromoted`](/keys/voxunitpromoted/) is the EVA line for either rise. With all five keys unset, a promotion makes no sound and does not flash.

A flash lasts the number of frames its timer gives. The promoted object's brightness alternates while the count runs, changing every two frames. The sound plays from the object's position, so it fades with distance from the view; [Placed sounds](/systems/sound-effects/#placed-sounds) covers that. The EVA line joins the speech queue described in [EVA speech](/systems/eva-speech/).

## Carrying rank between objects

A vehicle keeps its experience when it deploys into a building, and the building passes that experience back when it undeploys. Capturing an object also preserves its rank and any abilities or elite weapon that rank grants.

Passengers who survive a destroyed transport keep their ranks. Crew created when a vehicle or building is destroyed start at rookie.

## Rank display

Veteran and elite objects display different rank insignia beside them, even when unselected. Buildings display them too. The insignia sits farther from vehicles, aircraft, and buildings than from infantry.

Ranks are visible to allies, players spying on the owner, and [observers with full map visibility](/systems/observers/). Shroud, fog, or an undetected cloak hides the insignia along with the object. The healer cross follows the same visibility rules.

The insignia uses `PIPS.SHP`. A below-rookie object uses the frame after the last defined pip; there is no dedicated insignia for that rank.
