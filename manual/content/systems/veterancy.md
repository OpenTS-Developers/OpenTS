---
title: Veterancy and promotion
summary: "Promotes an object through veteran and elite rank as it destroys value, unlocking per-type abilities and an elite weapon."
category: combat-targeting
keys:
  - Armory
  - CrateRadius
  - Elite
  - EliteAbilities
  - IRepairRate
  - InitialVeteran
  - Trainable
  - VeteranAbilities
  - VeteranArmor
  - VeteranCap
  - VeteranCombat
  - VeteranLevel
  - VeteranROF
  - VeteranRatio
  - VeteranSight
  - VeteranSpeed
related:
  - type: system
    id: drop-pods
  - type: format
    id: teamtypes
  - type: enum
    id: CrateType
---

## Ranks

Each runtime instance carries a single experience figure, and its rank is read directly off that number. The four bands are exclusive:

| Rank | Experience |
| --- | --- |
| Below rookie | less than `0` |
| Rookie | `0` up to but not including `1` |
| Veteran | `1` up to but not including `2` |
| Elite | `2` and above |

Every object is created at rookie with an experience of `0`. The thresholds `1` and `2` are fixed in the engine; the rules govern how fast the figure climbs, not where the boundaries lie. The figure belongs to the object and not to its type, so two objects built from the same rules section sit at whatever rank each has earned or been handed.

Among the rules and AI settings, [`VeteranLevel=0`](/keys/veteranlevel/) is the only source of negative experience, assigning `-0.25`; a map's placed-object records can also write the experience figure directly, at any value. The state costs the object nothing in combat: a below-rookie object holds no abilities, exactly like the rookie it sits under. The state shows in three places only — the insignia it draws, the veterancy crate that steps it up to rookie, and the armory, which sends it out a veteran where any other occupant would leave elite.

## Earning experience

Experience is credited in one place: when an object is destroyed, whatever dealt the fatal damage receives `victim cost / (killer cost * VeteranRatio)`. Reaching veteran therefore means destroying [`VeteranRatio`](/keys/veteranratio/) times the killer's own cost in enemy value, and reaching elite means destroying twice that. The figure taken from each type is its [`Cost`](/keys/cost/#scope-aircrafttype), not its [`Points`](/keys/points/). A structure that hands out a [free unit](/keys/freeunit/) has that unit's price subtracted from its own and then added straight back here, so a kill is still worth the structure's written price — unless the unit is priced at or above the structure, where the subtraction floors at nothing and the structure is worth the unit's price instead. The division has no zero guard: a `VeteranRatio` of `0`, or a killer type whose `Cost` is `0`, divides by zero.

:::caution[Price multipliers change nothing about how fast a unit promotes]
Both figures in the fraction are priced through the house that lost the object. The victim's value is its own type's price scaled by that house's [multipliers](/keys/cost/#what-a-house-pays), and the killer's cost is its type's price scaled by those same multipliers — the killer's own house is never asked. The two scalings cancel, so what a kill is worth comes out of the two written `Cost` figures alone, and a house paying double for everything it builds promotes its units on exactly the tally a house paying half does.
:::

Only a killer whose type is [`Trainable=yes`](/keys/trainable/) accumulates anything; the check is on the killer's own type, and it gates earning alone. An object that was handed a rank some other way keeps every benefit of that rank whether or not its type is trainable. BuildingTypes start untrainable and must set the key explicitly to earn from their own kills.

The house that lost the object also tests whether it considers the killer allied. An allied killer receives no experience, while the remaining score, loss, and trigger bookkeeping for the destroyed object still runs.

A killer is credited separately for each occupant that dies with a transport it destroys, and a vehicle that crushes something is credited as that object's killer.

:::caution[Capturing awards no experience]
A captured object is booked as a kill with no killer attached, and the formula runs only where there is a killer, so nothing gains experience from a capture. The points are a separate step alongside that kill record: the capturing house is credited the captured object's cost. Selling a building and letting a unit sink out of the world are booked as killerless kills in the same way, and those two credit no points to anyone.
:::

Nothing decays experience and nothing resets it on ownership change, so a rank is held until something explicitly reassigns it.

### The experience ceiling

Every credited kill clamps the result to [`VeteranCap`](/keys/veterancap/) after adding to it. The clamp runs only on this path — the promotion sources below write a rank directly and ignore the ceiling.

:::caution[The engine default keeps elite out of reach]
With no `VeteranCap` in the rules the ceiling is `1`, which is exactly the veteran threshold: kills can promote an object to veteran and no further. Raising it to `2` is what makes elite reachable through combat.
:::

:::danger[A low ceiling demotes elites that keep fighting]
The clamp is applied to the total, not to the increment, so an elite object whose experience already sits above the ceiling is pulled back down the moment it earns a credited kill. Under a `VeteranCap` of `1`, an elite trainable object drops to veteran on its next kill, losing its elite weapon and its [`EliteAbilities`](/keys/eliteabilities/) with it.
:::

## Promotion without kills

Five settings-driven paths set a rank with no experience earned, and a map's placed-object records can write an experience figure directly besides. None of them consults `VeteranCap` or the kill formula. The veterancy crate alone consults `Trainable`.

| Source | Result |
| --- | --- |
| [Veterancy crate](/reference/enums/crate/) | Every `Trainable=yes` object on the ground within [`CrateRadius`](/keys/crateradius/) of the crate rises one rank, repeated as many times as the crate's `[Powerups]` data field says. |
| [`Armory=yes`](/keys/armory/) building | The infantry inside is promoted once the building's servicing counter runs out. |
| [Drop Pods superweapon](/systems/drop-pods/#drop-pods-superweapon) | Each delivered passenger is created elite. |
| TeamType [`VeteranLevel`](/keys/veteranlevel/) | Every member created for the team takes the rank the value names. |
| [`InitialVeteran=yes`](/keys/initialveteran/) | The units and infantry drawn from the random starting selection of a skirmish or multiplayer match are created elite. |

The crate is the only path that steps a rank rather than assigning one: it lifts a below-rookie object to rookie, a rookie to veteran, and a veteran to elite, and leaves an elite where it is. The other four assign a rank outright — three write a fixed rank over whatever the object was carrying, and the armory picks between veteran and elite by the occupant that walked in.

:::caution[A veterancy crate promotes every trainable object nearby]
The radius sweep tests position and `Trainable`, but not ownership. Enemy and neutral objects with `Trainable=yes` standing inside `CrateRadius` are promoted alongside the collector's own, and trainable buildings within the radius are promoted too.
:::

:::caution[An armory skips the veteran rank]
The armory raises a below-rookie occupant to veteran and everything else straight to elite, so a rookie infantry that walks in comes out elite and a veteran gains nothing it could not have had for free. The enter cursor is offered only while the infantry is not already elite, and each admission spends one point of the building's [`Ammo`](/keys/ammo/) pool, which an armory never restocks.
:::

The armory delay comes from [`IRepairRate`](/keys/irepairrate/), which the hospital shares.

## What a rank changes

Rank by itself changes almost nothing. Except for the elite weapon and the cell-wide scatter noted below, every benefit is gated on an ability, and the rank only decides which of the type's two ability lists is consulted. A veteran of a type that names no abilities gains a rank insignia and nothing else.

### Abilities

Each object type carries two ability sets, [`VeteranAbilities`](/keys/veteranabilities/) and [`EliteAbilities`](/keys/eliteabilities/), each a comma-separated list of tokens. A veteran draws on `VeteranAbilities`. An elite draws on both lists together, so an ability named in `VeteranAbilities` continues to apply after the second promotion. Nothing below veteran holds any ability at all, and `EliteAbilities` is never consulted below elite rank.

```ini title="rules.ini"
[MYINF] ; example InfantryType
VeteranAbilities=FIREPOWER,ROF
EliteAbilities=SELF_HEAL,FEARLESS
```

A veteran `MYINF` hits harder and reloads faster. An elite one keeps both of those and adds self-repair and immunity to fear.

The eighteen accepted tokens are matched without regard to letter case, and the table gives each one's effect on an object that holds it. They fall into three kinds: ten hand the object a flag its type could have carried from the start, five scale a figure from the rules, and `SCATTER`, `RADAR_INVISIBLE` and `GUARD_AREA` change behavior with no rules figure behind them.

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
| `SELF_HEAL` | The object repairs itself while at or below the yellow health threshold, as with [`SelfHealing=yes`](/keys/selfhealing/). |
| `EXPLODES` | Death produces the violent explosion of [`Explodes=yes`](/keys/explodes/). |
| `RADAR_INVISIBLE` | The object is kept off the local player's radar unless a sensor contact picks it up. |
| `SENSORS` | An adjacent enemy cloaked object shimmers, as with [`Sensors=yes`](/keys/sensors/). |
| `FEARLESS` | Infantry accumulate no fear, as with [`Fearless=yes`](/keys/fearless/). |
| `C4` | Infantry attack buildings by sabotage, as with [`C4=yes`](/keys/c4/). |
| `TIBERIUM_HEAL` | Standing in Tiberium repairs the object, as with [`TiberiumHeal=yes`](/keys/tiberiumheal/). |
| `GUARD_AREA` | An idle armed vehicle, and an idle human-owned infantry, takes the Guard Area mission instead of Guard. A member of a team is unaffected, and so is a computer-owned infantry, whose idle handling never reads the ability. |
| `CRUSHER` | A vehicle crushes crushable objects and overlays, as with [`Crusher=yes`](/keys/crusher/). |

An unrecognized token is discarded without complaint, so a misspelling produces a rank with fewer benefits and no diagnostic.

:::caution[A space after a comma silences the token]
Leading and trailing whitespace is stripped from the value as a whole, not from each token, so `VeteranAbilities=FIREPOWER, ROF` registers `FIREPOWER` alone. Write the list without spaces around the separators.
:::

:::caution[Only the first 127 characters are parsed]
The remainder of a longer value is discarded, and a token cut in half by that boundary is dropped as unrecognized. All eighteen tokens with their separating commas run to 163 characters, so a complete list cannot be assigned in one setting.
:::

An ability list replaces rather than merges: a later rules layer that carries the key starts from an empty set and keeps only what it names. Omitting the key in that layer leaves the earlier list in force — as does assigning an empty value, which is discarded when the file is read, so a list cannot be cleared once set.

:::caution[An elite object makes its whole cell scatter]
When fire comes in at a cell holding an elite object, every occupant of that cell scatters — including objects that are neither elite nor carrying `SCATTER`. This is the one effect that reads rank directly instead of going through an ability.
:::

### Rank multipliers

Each figure is fetched at the moment the statistic is needed, and each states a fraction to add rather than a finished multiplier:

| Setting | Applied as |
| --- | --- |
| [`VeteranCombat`](/keys/veterancombat/) | Damage is multiplied by the value plus one, after the house and object firepower biases. |
| [`VeteranSpeed`](/keys/veteranspeed/) | Speed is multiplied by the value plus one, after the house ground-speed bias. |
| [`VeteranSight`](/keys/veteransight/) | Sight range is multiplied by the value plus one. |
| [`VeteranArmor`](/keys/veteranarmor/) | Incoming damage is divided by the value plus one. |
| [`VeteranROF`](/keys/veteranrof/) | Reload delay is divided by the value plus one. |

`VeteranArmor` and `VeteranROF` are divisors, so raising them lowers the number they act on: `1` halves damage taken and halves the reload delay, while `0` leaves both untouched. The other three are multipliers, where `1` doubles the statistic and `0` leaves it untouched. All five default to `1`.

A sonic weapon and a weapon that fires through the fire particle system never receive the firepower bonus, because their damage figure is zeroed before the veteran step is reached. The reload bonus is skipped on a sonic weapon, and on a weapon whose spark, fire, or railgun particle system is attached; within a burst it is skipped as well, so only the delay that follows the last shot of a burst is shortened.

### The elite weapon

An elite object fires the WeaponType named by [`Elite`](/keys/elite/) wherever its primary weapon would otherwise be used — target selection, range tests, reload delay, and the shot itself all resolve through the substituted weapon slot. The secondary weapon is never swapped, and a veteran gets no swap at all. A type that leaves `Elite` unset falls back to its primary, so the substitution is invisible on types that do not define one.

The elite weapon slot reuses the primary's art keys, [`PrimaryFireFLH`](/keys/primaryfireflh/), [`PBarrelLength`](/keys/pbarrellength/), and [`PBarrelThickness`](/keys/pbarrelthickness/), so the elite weapon fires from the same muzzle offset as the weapon it replaces.

:::caution[An upgrade plug outranks the elite weapon]
A building resolves its weapon through its plugs before its own type — a plug being a structure type carrying [`PowersUpBuilding=`](/keys/powersupbuilding/), installed into one of the host's [upgrade slots](/keys/upgrades/). When a plug supplies a weapon in the weapon slot being asked for, that weapon is used and the elite substitution never runs, so the building fires the plug's primary even at elite rank.
:::

### When a promotion takes effect

Nothing about a rank is cached. Each multiplier is fetched from the rules and each ability list from the type at the moment it is needed, so a promotion applies from the next shot, the next damage event, or the next movement step.

Sight is the exception. The wider radius is computed inside the routine that reveals terrain, so a promoted object's revealed area grows only when it next looks, not at the moment of promotion.

## Carrying rank between objects

Deploying carries experience across in both directions: a construction vehicle that deploys hands its experience to the building it becomes, and a building that undeploys hands it back to the vehicle. Change of ownership leaves experience untouched, so a captured building keeps the rank it was carrying, along with the abilities and elite weapon that rank unlocks.

Passengers that get clear of a wrecked transport are the same objects they were and keep their own ranks. The crew that escapes a destroyed vehicle or building is a freshly created infantry object and starts at rookie.

## Rank display

A veteran and an elite each draw their own insignia from the hard-coded `PIPS.SHP`, placed beside the object and pushed further out for anything that is not infantry. The insignia is drawn with the object's condition indicator and only for a viewer allied to its owner or spying on that house, so an enemy's ranks stay hidden. A building that acquired a rank by deploying draws one as well. An object below rookie draws the frame immediately after the last named pip rather than an insignia designed for that state.
