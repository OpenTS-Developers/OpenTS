---
title: Crates
summary: "Places pickup crates on the map and gives one result to the infantry or vehicle that collects each crate."
category: maps-scenarios
keys:
  - ArmorCrateStacks
  - CarriesCrate
  - Crate
  - CrateGoodie
  - CrateImg
  - CrateMaximum
  - CrateMinimum
  - CrateMoneyBonus
  - CrateRadius
  - CrateRegen
  - CrateTrigger
  - Crates
  - FirepowerCrateStacks
  - HealCrateSound
  - SilverCrate
  - SoloCrateMoney
  - TrainCrate
  - TruckCrate
  - UnitCrateType
  - WoodCrate
  - WoodCrateImg
related:
  - type: enum
    id: CrateType
  - type: system
    id: veterancy
  - type: system
    id: cloaking
  - type: system
    id: tiberium
  - type: system
    id: superweapons
  - type: event
    id: TEVENT_PICKUP_CRATE
  - type: event
    id: TEVENT_PICKUP_CRATE_ANY
  - type: action
    id: TACTION_TOGGLE_TRAIN_CARGO
---

A crate is an overlay. Any OverlayType with [`Crate=yes`](/keys/crate/) is collected as a crate. Crates the engine places itself always use the overlay type named by [`WoodCrateImg`](/keys/woodcrateimg/). In a campaign, `WoodCrateImg` and [`CrateImg`](/keys/crateimg/) also decide which result a crate gives.

Outside a campaign, the engine places random crates, draws each crate's result from weighted shares, and replaces crates as they expire or are collected. A campaign does none of this: its crates come only from the map and from destroyed vehicles, and each gives a fixed result.

## Placing crates

### At scenario start

Outside a campaign, when crates are enabled for the match, random crates are placed once as the scenario finishes loading. The number placed is the larger of [`CrateMinimum`](/keys/crateminimum/) and the number of human players, capped at [`CrateMaximum`](/keys/cratemaximum/). Both keys belong in `[CrateRules]` in `rules.ini`.

Network games and spawned matches record the number of human players. A skirmish started from the game's menus does not, so it places `CrateMinimum` crates. A network game played earlier in the same session can leave a larger count behind.

Each random crate occupies one of 256 tracking slots until it is collected or expires. When all 256 slots are in use, no further random crate is placed, whatever `CrateMaximum` says.

### Crates drawn into a map

A map can draw crates in its overlay layer. They appear in every match, including campaigns and matches with crates switched off. As with every other overlay, a crate drawn outside the playfield is dropped.

A drawn crate appears exactly where it was drawn, whatever occupies the cell or its terrain type. The only exception is the slope: a crate appears only on flat ground or on one of the four standard ramps. A standard ramp slopes toward one of the four map directions and has two of the cell's corners raised. Corner, steep and double-ramp slopes never hold a crate, however it arrives.

A drawn crate takes no tracking slot, so it never expires. It stays on the map until something collects it.

### Where a random crate can land

Placing a random crate makes up to 1000 attempts. Each attempt draws a random cell anywhere on the map, including the border outside the playable area, then goes through these steps:

1. If the cell lies outside the playable area, the attempt fails and the next attempt draws a new cell.
2. If the cell holds an overlay, the engine draws a fresh random cell, then takes a nearby cell that is unoccupied, lies in the playable area and can be crossed by tracked vehicles. It repeats this until the cell has no overlay or has one that allows a crate. The new cell is not near the rejected one.
3. The engine creates the `WoodCrateImg` overlay on that cell. It never places `CrateImg`.
4. The crate appears only if the cell passes **all of** these tests, in this order:

   - the cell is flat or has one of the four standard ramps;
   - it holds no infantry, vehicle, building, landed aircraft, or terrain object such as a tree;
   - it has no wall;
   - tracked vehicles can cross its land type;
   - the overlay already in the cell, if there is one, does not set [`Overrides=yes`](/keys/overrides/).

In step 2, an overlay allows a crate only when its type sets [`BuildableOver=yes`](/keys/buildableover/), it is not a wall, and the cell's land type is passable to the `Foot` or the `Float` speed type. Any other overlay forces a redraw, including Tiberium, veins, walls, bridges and an existing crate. A crate placed on such a cell replaces the overlay, unless the overlay also sets `Overrides=yes`, in which case step 4 rejects the cell.

The wall test in step 4 never fails, because step 2 always moves away from a wall.

:::caution[A rejected cell still uses up a crate]
If the cell fails a step 4 test, no crate appears, but the placement still counts as a success. The tracking slot and lifetime timer start as usual, so the match has one crate fewer until that timer expires and a replacement is placed elsewhere.

Random crates never land on water for this reason: tracked vehicles cannot cross water, so a crate placed there is rejected. A crate drawn into the map on water stays, because drawn crates skip these tests.
:::

### How long a crate lasts

A random crate's lifetime is chosen when it is placed. It is a random time between half of [`CrateRegen`](/keys/crateregen/) and twice `CrateRegen`, in minutes of game time. The shipped rules set `CrateRegen=3`, so a crate lasts between 1.5 and 6 minutes. With `CrateRegen=10`, it lasts between 5 and 20 minutes.

:::note[CrateRegen is not the average lifetime]
The average lifetime is 1.25 times `CrateRegen`. Raising `CrateRegen` lengthens both the shortest and the longest lifetime.
:::

Outside a campaign, while crates are enabled for the match, a crate that reaches the end of its lifetime is removed and a new random crate is placed.

Collecting a crate also places a new random crate, outside a campaign, when crates are enabled for the match and [`Crates`](/keys/crates/) in `[MultiplayerDefaults]` is `yes`. This applies to every collected crate, including one drawn into the map or dropped by a vehicle.

:::caution[Crates=no in the rules still stops pickup replacements]
In `rules.ini`, `Crates` in `[MultiplayerDefaults]` only sets the starting state of the match's crate option, which the match setup then overwrites. With `Crates=no` in the rules and crates switched on for the match, collected crates are not replaced, but expired crates still are. Switching crates off for the match stops both kinds of replacement, whatever the rules say.

A map's `[MultiplayerDefaults]` section does not change the match option. `Crates=no` there also stops pickup replacements.
:::

### Crates dropped by destroyed vehicles

A destroyed vehicle whose type sets [`CarriesCrate=yes`](/keys/carriescrate/) can drop a `WoodCrateImg` crate. The crate lands on a nearby cell in the playable area that is unoccupied, has no overlay, and can be crossed by tracked vehicles. If no such cell is found, nothing drops.

The map's `[Basic]` section decides whether drops happen. A type with [`IsTrain=yes`](/keys/istrain/) drops a crate only when [`TrainCrate`](/keys/traincrate/) is enabled, and every other type only when [`TruckCrate`](/keys/truckcrate/) is enabled. The [Toggle Train Cargo](/mapping/actions/taction-toggle-train-cargo/) trigger action changes the train setting during a mission.

A dropped crate takes no tracking slot, so, like a crate drawn into the map, it never expires.

## Collecting a crate

Infantry, walkers, hovercraft and driven vehicles collect a crate when they move into its cell. These are the walking, mech, hovering and driving locomotors. Objects with any other locomotor never collect a crate: jump-jet infantry and flying aircraft pass over the cell, and buildings never move onto one.

A crate can be collected anywhere its collector can move. Crates placed by the engine always lie inside the playable area, so any crate in the map border was drawn there by the map.

Some objects refuse to enter a crate's cell at all. The table shows which, by whether a human plays the object's house and whether the match is a campaign:

| The collector | In a campaign | Outside a campaign |
| --- | --- | --- |
| A vehicle of a human-played house | Enters and collects | Enters and collects |
| Infantry of a human-played house | Enters and collects | Enters and collects |
| A vehicle of a house that is not human-played | Refuses the cell | Enters and collects |
| Infantry of a house that is not human-played | Refuses the cell | Refuses the cell |

Outside a campaign, a house whose country sets [`MultiplayPassive=yes`](/keys/multiplaypassive/) never collects a crate. Its vehicles still drive over crates, leaving them in place, and its infantry refuse crate cells as the table shows. In a campaign, `MultiplayPassive` does not affect crates.

### Springing the crate trigger

When the overlay type sets [`CrateTrigger=yes`](/keys/cratetrigger/), collecting the crate fires two trigger events:

- [Pickup Crate](/mapping/events/tevent-pickup-crate/) springs on the collector's tag, if it has one, before the result is chosen. If that trigger destroys the collector, the crate stays on the map, gives no result, and Pickup Crate (any) does not fire.
- [Pickup Crate (any)](/mapping/events/tevent-pickup-crate-any/) fires on the next game frame.

Crates without `CrateTrigger=yes` fire neither event.

## The `[Powerups]` section

`[Powerups]` gives each crate result a share of the random draw, an animation, and one number that the result's effect uses. The entry names are the result tokens on the [crate result](/reference/enums/crate/) page, and [Crate powerups](/formats/powerups/) owns the syntax. If you already write `[Powerups]` entries, skip to [choosing the result](#choosing-the-result).

Each value is a comma list of `Share,Anim,Data`. The animation can be `<none>` or `none`; an AnimType name the rules do not register also gives no animation. The third field is optional. Written with a percent sign, it is divided by 100: `Armor=33,ARMOR,50%` stores `0.5`.

```ini title="rules.ini"
[Powerups]
Money=55,MONEY,2000     ; share 55, plays the MONEY animation, pays at least 2000 credits
Armor=33,ARMOR,2        ; share 33, plays ARMOR, halves incoming damage inside the radius
Explosion=38,<none>,500 ; share 38, no animation, 500 raw damage per blast
Veteran=15,VETERAN,1    ; share 15, plays VETERAN, one promotion step per object
```

:::caution[List every result you want to keep]
A `[Powerups]` section in any rules file or map sets every result it omits to a share of `0` with no animation. Only the omitted result's third field keeps its value. The four rows above are an example, not the shipped section: on their own they would disable the other fifteen results.
:::

:::danger[Keep the total share above zero]
If every share is `0`, about half of the crates collected outside a campaign read past the end of the result list, which can crash the game. The other half pay money.
:::

## Choosing the result

### In a campaign

In a campaign, a crate's result depends only on its overlay type, and the shares in `[Powerups]` are ignored. The overlay is tested in this order:

1. An overlay matching `CrateImg` gives the [`SilverCrate`](/keys/silvercrate/) result.
2. An overlay matching `WoodCrateImg` gives the [`WoodCrate`](/keys/woodcrate/) result, replacing the result from step 1.
3. Any other crate overlay gives money.

A campaign crate that is collected is not replaced. A `Squad` result becomes money, as described under [results without effect](#settings-and-results-without-effect).

:::caution[Give SilverCrate its own overlay type]
When `WoodCrateImg` and `CrateImg` name the same overlay type, step 2 always overrides step 1, so every crate gives the `WoodCrate` result and `SilverCrate` never applies. The shipped rules name `CRATE` for both, and `CRATE` is the only overlay type with `Crate=yes`. To use `SilverCrate`, add a second OverlayType with `Crate=yes`, name it in `CrateImg` only, and draw it into the map.
:::

### Outside a campaign

Outside a campaign, the result is drawn at random, weighted by the shares in `[Powerups]`. Each result's chance is its share divided by the total of all shares, so raising one share lowers the chance of every other result.

The drawn result is then replaced by a free MCV, delivered through the `Unit` result, when the collector's house meets **all of** these conditions:

- it owns no buildings;
- it has more than 1500 credits available;
- it owns no vehicle of any [`BaseUnit`](/keys/baseunit/) type;
- bases are enabled for the match.

Six results are then converted to money. The conversions run after the MCV override, so a house with more than 50 vehicles receives money in place of its free MCV.

| Drawn result | Converted to money when |
| --- | --- |
| `Unit` | the collector's house owns more than 50 vehicles |
| `Squad` | always, in campaigns too |
| `Armor` | the collector's armor multiplier was already changed by an earlier crate, unless [`ArmorCrateStacks=yes`](/keys/armorcratestacks/) |
| `Speed` | the collector's speed multiplier was already changed |
| `Firepower` | the collector's firepower multiplier was already changed, unless [`FirepowerCrateStacks=yes`](/keys/firepowercratestacks/); or the collector has no primary weapon |
| `Cloak` | the collector can already cloak |

:::caution[The conversions check only the collector]
`Armor`, `Speed`, `Firepower` and `Cloak` affect every object inside [`CrateRadius`](/keys/crateradius/), but the conversion checks only the collector. A firepower crate collected by an already-boosted vehicle becomes money, even when a dozen unboosted vehicles stand inside the radius.
:::

## What each result does

The third field of a result's `[Powerups]` row is the only per-result number the engine reads, and most results ignore it.

| Result | What the third field sets |
| --- | --- |
| `Money` | The minimum credits paid outside a campaign; a random bonus of up to [`CrateMoneyBonus`](/keys/cratemoneybonus/) is added |
| `Unit` | The minimum credits paid outside a campaign when the vehicle cannot be placed |
| `Explosion` | Raw damage of the direct hit on the collector and of each of the five blasts |
| `Napalm` | Raw damage of the direct hit on the collector and of the blast |
| `Gas` | Raw damage applied to each of the nine cells |
| `Veteran` | How many promotion steps each object takes |
| `Armor` | The armor multiplier |
| `Speed` | The speed multiplier |
| `Firepower` | The firepower multiplier |
| Every other result | Not read |

### Money and free units

Outside a campaign, a money crate pays a random amount between the `Money` row's third field and that figure plus [`CrateMoneyBonus`](/keys/cratemoneybonus/) credits, inclusive. In a campaign it pays [`SoloCrateMoney`](/keys/solocratemoney/) instead. If `SoloCrateMoney` is `0`, a campaign crate uses the random range.

In a campaign, money from a crate collected by any house the local player controls goes to the player's house, not to the collecting house.

The unit result chooses a vehicle type in these steps. A later step can replace an earlier choice. Steps 1 and 2 use the house's preferred entry of a list: the first entry that the country the house acts as may own, or the list's first entry if that country may own none.

1. If the MCV override applied, the house's preferred `BaseUnit` entry.
2. If no type is chosen yet, and the house owns a structure of any [`BuildRefinery`](/keys/buildrefinery/) type but no vehicle of any [`HarvesterUnit`](/keys/harvesterunit/) type: the house's preferred `HarvesterUnit` entry.
3. If [`UnitCrateType`](/keys/unitcratetype/) names a type, that type, replacing any earlier choice.
4. If no type is chosen yet, a random vehicle type that meets **all of** these conditions:

   - it sets [`CrateGoodie=yes`](/keys/crategoodie/);
   - the country the collector's house acts as may own it;
   - **Any of:** bases are enabled for the match, or it is not a `BaseUnit` type.

If no vehicle type meets the step 4 conditions, the crate gives nothing: no vehicle and no money. It still plays the `Unit` row's animation.

:::caution[UnitCrateType replaces the MCV and harvester rescues]
Leave `UnitCrateType` unset to keep both rescues. A type named in `UnitCrateType` replaces the free MCV given to a house that has lost its base, and the free harvester given to a house that has lost its last one.
:::

The vehicle appears on the crate's cell or on a nearby cell it can occupy. Either way, the collector stops short of the crate's cell.

If the vehicle cannot be placed in either location, the crate pays money instead. Outside a campaign, the amount is drawn from the `Unit` row's third field up to `CrateMoneyBonus` credits more, not from the `Money` row.

### Results that sweep a radius

`Cloak`, `Veteran`, `Armor`, `Speed` and `Firepower` affect objects on the ground within [`CrateRadius`](/keys/crateradius/) of the center of the crate's cell. None of them checks ownership, so enemy and neutral objects inside the radius are affected along with the collector's side.

- `Cloak` makes each object able to cloak, which [cloaking and detection](/systems/cloaking/) then acts on.
- `Veteran` promotes each object whose type sets `Trainable=yes`, as many steps as its third field says. [Promotion without kills](/systems/veterancy/#promotion-without-kills) describes the steps.
- `Armor`, `Speed` and `Firepower` each set a multiplier on the affected objects.

The three multiplier results change only objects whose multiplier for that result is still exactly `1`. A second crate of the same kind therefore leaves an already-boosted object unchanged. `Speed` also skips buildings and aircraft.

`ArmorCrateStacks=yes` and `FirepowerCrateStacks=yes` in `[CrateRules]` drop that check for their result, so each crate multiplies the multiplier again. `Speed` has no such setting.

Each multiplier result stores its third field as written. Speed and firepower are multiplied by it, while incoming damage is divided by the armor value. An `Armor` value of `2` halves ordinary incoming damage, and `0.5` doubles it.

:::caution[Keep the Armor value above zero]
The `Armor` row's third field is not clamped. A value of `0` makes damage to every affected object divide by zero.
:::

### Results that reach the whole map

`HealBase` restores every object of the collector's house that is on the map to maximum strength. `CrateRadius` does not limit it. Passengers inside a transport are not healed. [`HealCrateSound`](/keys/healcratesound/) plays at the collector when the collector belongs to the local player.

`Darkness` covers the whole map in shroud again, and `Reveal` reveals the whole map. Both act for the player who controls the collecting house, including, in a campaign, a house the scenario marks [`PlayerControl=yes`](/keys/playercontrol/). Collected by any other house, either crate is used up and plays its animation without changing anyone's vision.

`Darkness` is the only crate result that restores shroud.

The `ICBM` result grants the collector's house a [one-time superweapon](/systems/superweapons/#one-shot-at-a-time); that section explains which weapon it is. A house that already holds that weapon gains nothing. The sidebar button appears only when the collector belongs to the local player.

### Damaging results

`Explosion` damages the collector with the [`C4Warhead`](/keys/c4warhead/) warhead. It then sets off five blasts of the same damage and warhead at random points within two cells of the crate. Each blast has its own explosion animation and lighting flash, and can set off chain reactions.

`Napalm` damages the collector with the [`FlameDamage`](/keys/flamedamage/) warhead. It then sets off a blast of the same damage halfway between the crate's cell and the collector. The blast plays the first AnimType the rules register, whatever its name, and `[Powerups]` cannot change it.

`Gas` applies its damage with the warhead named `GAS` to the crate's cell and each of its eight neighbors. The damage has no attacker, so its kills earn no one experience, and it sets off no chain reactions. If the rules define no warhead named `GAS`, the result does nothing.

### Tiberium

`Tiberium` places one growth stage of a random [Tiberium](/systems/tiberium/) type on the crate's cell, and at 10 to 20 random points within three cells of it. Two points can land on the same cell. A point that lands on a cell already holding that Tiberium grows it one stage. A cell that cannot take it is left unchanged.

The type is drawn from the registered Tiberium types, except that a draw of the second entry in `[Tiberiums]` becomes the first entry. The second type therefore never comes from a crate, and the first type is twice as likely as each of the others.

## Settings and results without effect

Three result tokens have no effect: `Invulnerability`, `IonStorm` and `Pod`. Drawing one uses up the crate and plays that row's animation; nothing else happens. `Pod` shares its name with the [Drop Pods](/systems/drop-pods/) superweapon but has no connection to it.

`Squad` always becomes money before its effect runs, so its share acts as extra weight on money. The payout uses the `Money` row's third field.

[`FreeMCV`](/keys/freemcv/) is read from `[CrateRules]` but has no effect. The [free MCV override](#outside-a-campaign) applies whenever its conditions hold, whatever `FreeMCV` says.

## What a crate leaves behind

The result's animation, named by the second `[Powerups]` field, plays slightly above the center of the crate's cell. It comes from the result that actually ran, so a converted crate plays the animation of what it became: a wasted firepower crate plays the money animation. A unit crate whose vehicle is placed plays no animation.

`Armor`, `Speed` and `Firepower` each play one EVA line per crate, and only when at least one affected object belongs to the local player.

When selected, an object whose armor or firepower multiplier is above 1 shows a different selection bracket. So does an infantry unit or vehicle whose speed multiplier is above 1. That bracket is the only lasting sign of the three multiplier crates. Buildings and [`IsCoreDefender=yes`](/keys/iscoredefender/) vehicles show a pip bar in place of a bracket, so they never show it.

A veterancy crate's effect shows as the rank insignia, and a cloak crate's effect shows when the object cloaks.
