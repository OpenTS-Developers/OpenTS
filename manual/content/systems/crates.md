---
title: Crates
summary: "Scatters pickup crates over the map and gives a single result from the crate table to the infantryman or vehicle that walks, drives or hovers onto one."
category: maps-scenarios
keys:
  - CarriesCrate
  - Crate
  - CrateGoodie
  - CrateImg
  - CrateMaximum
  - CrateMinimum
  - CrateRadius
  - CrateRegen
  - CrateTrigger
  - Crates
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

A crate is an overlay, not an object. Any OverlayType carrying [`Crate=yes`](/keys/crate/) counts as one. [`WoodCrateImg`](/keys/woodcrateimg/) names the overlay the engine places, and `WoodCrateImg` and [`CrateImg`](/keys/crateimg/) together name the two it recognizes as its own. Whether crates appear at all, how a result is chosen, and whether a collected crate is replaced all turn on whether the match is a campaign.

## Placing crates

### At scenario start

Random placement runs once, at the end of scenario setup, and only outside a campaign. Its count is the larger of [`CrateMinimum`](/keys/crateminimum/) and the number of human players the match was set up with, clamped down to [`CrateMaximum`](/keys/cratemaximum/). Network game setup is the only path that records that player count.

Each of those crates goes through the placement search below, and each one that succeeds takes a tracking slot. There are 256 slots. A placement request that finds every slot occupied places nothing, so 256 is the ceiling on tracked crates regardless of `CrateMaximum`.

### Crates drawn into a map

A map's overlay layer can carry crates, and they survive only in a campaign. Outside one, every crate found in the overlay layer is discarded as the map is read. Cells outside the playfield are dropped for all overlays alike.

A crate kept this way is placed while the scenario is still loading, which suppresses the legality test described below: it lands exactly where the map author drew it, on any ground that is flat or carries one of the four standard ramps — the slopes that fall away toward one of the map's four directions with two of the cell's corners raised. The corner, steep and double ramp shapes lie outside that set, and no crate is placed on one however it arrives. A kept crate is also never registered in a tracking slot, so no timer is ever attached to it and nothing expires it. It stays where it was drawn until something collects it.

### Where a random crate can land

Each placement attempt draws a uniformly random cell from the map rectangle — the upright square of cells that encloses the playfield — and applies no legality test to that draw at all. Up to 1000 attempts run before the request is abandoned. An attempt then proceeds as follows.

1. The cell must lie inside the playable area. A cell outside it fails the attempt outright, and the next attempt draws again.
2. While the cell already carries an overlay of any kind, a fresh random cell is drawn and moved to a nearby cell passable to tracked movement, chosen from the candidates around it rather than the closest one. Tiberium, veins, walls, bridges, and an existing crate all force that redraw, and because the redraw starts from a new random cell it does not stay near the rejected one.
3. The overlay is created at the surviving cell — always the one named by `WoodCrateImg`. `CrateImg` is never placed by the engine; it exists for map authors and for the campaign result lookup.
4. The overlay draws itself into the cell on **all of**, in this order:

   - the cell is flat or carries one of the four standard ramps;
   - it holds no infantry, vehicle, building or landed aircraft;
   - it carries no wall;
   - its land type is passable to tracked movement;
   - the overlay already in the cell, if there is one, is not of a priority type.

   The wall term and the priority term cannot decide anything here. Step 2 has already redrawn away from every cell that carries an overlay of any kind, so the cell arriving at step 4 carries none at all, wall or otherwise.

:::caution[A rejected cell still consumes a crate]
Step 4 is the only test of terrain and occupancy on the path a crate normally takes, it runs inside the overlay rather than in the placement routine, and its failure is not reported back. The tracking slot records the location and starts its timer regardless, so the attempt counts as a success while no crate appears anywhere on the map. That slot later expires and rolls a replacement in the ordinary way. This is also why no engine-placed crate lands on water: water is not passable to tracked movement, so a crate rolled onto it is silently dropped. A crate the map author drew there survives, because a map's own crates skip this test.
:::

### How long a crate lasts

A tracked crate's lifetime is drawn at placement, uniformly, between half of [`CrateRegen`](/keys/crateregen/) minutes and twice it. With the engine default of `10` a crate lives between 5 and 20 minutes; at `CrateRegen=3` it lives between 1.5 and 6 minutes.

:::caution[CrateRegen is the bottom of the range, not the period]
A crate lasts somewhere between half `CrateRegen` and twice it, so the setting is the lower quarter-point of the range rather than its middle, and the average life is a quarter longer than the figure written. Raising it stretches both ends at once.
:::

Outside a campaign, and while crates are enabled for the match, every logic frame sweeps the tracking slots. An expired slot has its crate removed and a fresh random crate placed. Collecting a crate also places a replacement when both the match option and [`Crates`](/keys/crates/) in the rules are enabled.

:::caution[The rules can still suppress pickup replacements]
`Crates=yes` seeds the match setting that the game setup screen then overwrites, so the two can end up opposed. With `Crates=no` in the rules and crates switched on for the match, collected crates are not replaced although the expiry sweep keeps working. Switching crates off for the match suppresses both replacement paths whatever the rules say.
:::

### Crates dropped by destroyed vehicles

A destroyed vehicle of a [`CarriesCrate=yes`](/keys/carriescrate/) type drops a wood crate on a nearby cell that lies inside the playable area, is passable to tracked movement, and carries no overlay. The scenario decides whether the drop happens at all: an [`IsTrain=yes`](/keys/istrain/) type is gated on [`TrainCrate`](/keys/traincrate/) and every other type on [`TruckCrate`](/keys/truckcrate/). The [Toggle Train Cargo](/mapping/actions/taction-toggle-train-cargo/) trigger action flips the train setting mid-mission.

A crate dropped this way takes no tracking slot, so like a map-authored crate it never expires.

## Collecting a crate

A crate is collected as infantry, a walker, a hovercraft or a driven vehicle commits to entering its cell — the walking, mech, hovering and driving locomotors, and no others. Nothing else reaches that check: an infantryman on a jump jet crosses the cell without collecting, so does an aircraft in flight, and a building never moves onto one at all.

Collection applies no test of its own against the map's regions, so a crate standing anywhere in the playfield can be picked up, the map's border included. Every path the engine puts a crate down on is confined to the playable area, so the only crate ever standing in the border is one a map author drew there.

Whether the collector reaches the cell at all is settled earlier, by the movement test for its kind, and that test reads two things: whether the collector's house is played by a human, and whether the match is a campaign. The table gives every combination, with the passive-house override on the last row. What it shows is that outside a campaign the two kinds part company — computer vehicles drive onto crates that computer infantry refuse to walk onto.

| The collector | In a campaign | Outside a campaign |
| --- | --- | --- |
| A vehicle of a human-played house | Enters and collects | Enters and collects |
| Infantry of a human-played house | Enters and collects | Enters and collects |
| A vehicle of a house that is not human-played | Refuses the cell | Enters and collects |
| Infantry of a house that is not human-played | Refuses the cell | Refuses the cell |
| Anything owned by a house of a [`MultiplayPassive=yes`](/keys/multiplaypassive/) country | Nothing is overridden; read the row for its kind | Whatever reached the cell passes over the crate and collects nothing |

The passive override is applied inside the collection routine, once the collector is already on the cell, so it changes nothing for a kind the row above turned back. Outside a campaign that leaves a passive house's vehicles driving over crates they cannot pick up, while its infantry never reach one.

### Springing the crate trigger

When the overlay type also carries [`CrateTrigger=yes`](/keys/cratetrigger/), collection springs [Pickup Crate](/mapping/events/tevent-pickup-crate/) on the collector's own tag before anything else happens. If that trigger destroys the collector, the crate is left in place and no result is chosen. Collection also raises a scenario flag that the next logic pass turns into [Pickup Crate (any)](/mapping/events/tevent-pickup-crate-any/) for every general trigger, then clears.

## The `[Powerups]` section

This section carries the vocabulary every draw and conversion below is written in. Anyone already writing `[Powerups]` rows can skip to [choosing the result](#choosing-the-result).

`[Powerups]` gives each result a share, a result animation, and one number. Its entry names are the result tokens listed on the [crate result](/reference/enums/crate/) page, and each value is a comma list of `Share,Anim,Data`. The animation accepts `<none>`; an AnimType name the rules do not register resolves to no animation rather than failing. The third field is optional and may be written as a percentage, in which case it is divided by 100.

```ini title="rules.ini"
[Powerups]
Money=55,MONEY,2000     ; share 55, plays the MONEY animation, pays 2000 credits before the bonus
Armor=33,ARMOR,2        ; share 33, plays ARMOR, halves incoming damage inside the radius
Explosion=38,<none>,500 ; share 38, no animation, 500 raw damage per blast
Veteran=15,VETERAN,1    ; share 15, plays VETERAN, one promotion step per object
```

:::danger[A partial Powerups section erases every result it omits]
Each entry is read against a hard-coded fallback of `0,NONE` rather than against the value already in force. As soon as any loaded rules layer or map contains a `[Powerups]` section, every result that section does not list has its share set to `0` and its animation cleared; only the third field survives. Listing four results does not adjust four results, it disables the other fifteen. If the surviving shares total zero the draw comes out as 0 or 1: a 0 lands on the first entry and pays money, while a 1 walks past the last entry and reads one position beyond the result tables.
:::

## Choosing the result

### In a campaign

Campaign selection is fixed, with no weighted draw and no substitution beyond the `Squad` rewrite below. The money figure is preloaded from [`SoloCrateMoney`](/keys/solocratemoney/), and two tests then run in order: an overlay matching `CrateImg` selects [`SilverCrate`](/keys/silvercrate/), and an overlay matching `WoodCrateImg` selects [`WoodCrate`](/keys/woodcrate/). A crate overlay that matches neither setting yields the money result. No replacement crate is placed.

:::caution[Pointing both image settings at one overlay hides SilverCrate]
The two tests are sequential rather than exclusive, and the `WoodCrateImg` test runs second, so when both settings name the same OverlayType the wood result always overwrites the silver one and `SilverCrate` can never take effect. The shipped rules do exactly this: `WoodCrateImg` and `CrateImg` both name `CRATE`, the only overlay type that declares `Crate=yes`. Reaching `SilverCrate` requires a second OverlayType of its own with `Crate=yes`, drawn into the map and named by `CrateImg` alone.
:::

### Outside a campaign

Every result's share is summed, a number is drawn between 1 and that total, and the results are walked in their fixed order until the running total reaches the draw. Shares are absolute weights against that running total rather than percentages, so raising one entry lowers the odds of every other entry. The sum is recomputed from the live values on each collection.

An override then replaces the draw outright. The collector's house takes the unit result, whatever was drawn, on all of:

- it owns no buildings;
- it has more than 1500 credits available;
- it owns no vehicle of the [`BaseUnit`](/keys/baseunit/) type;
- bases are enabled for the match.

The override raises the flag that later turns that vehicle into an MCV, but it runs before the conversions below rather than after them, so a house holding more than 50 vehicles has the unit result turned straight back into money and never reaches the MCV the flag was raised for.

Six results are then converted to money. Five convert only when the collector would gain nothing from them; the sixth converts every time:

| Drawn result | Converted to money when |
| --- | --- |
| `Unit` | the collector's house owns more than 50 vehicles |
| `Squad` | always — its infantry test is overtaken by an unconditional rewrite |
| `Armor` | the collector's own armor multiplier has already been changed |
| `Speed` | the collector's own speed multiplier has already been changed, or the collector is an aircraft |
| `Firepower` | the collector's own firepower multiplier has already been changed, or the collector has no primary weapon |
| `Cloak` | the collector can already cloak |

:::caution[The conversions read only the collector]
`Armor`, `Speed`, `Firepower`, and `Cloak` all reach past the collector to the objects inside [`CrateRadius`](/keys/crateradius/) when they run, but the conversion above inspects the collector alone. A single already-boosted vehicle turns a firepower crate into money while a dozen un-boosted friendlies stand inside the radius.
:::

## What each result does

The third field of a result's `[Powerups]` row is the only per-result number the engine reads, and most results ignore it.

| Result | What the third field sets |
| --- | --- |
| `Money` | Credits paid, before a random bonus of up to 900 |
| `Unit` | Credits paid when no vehicle can be placed |
| `Explosion` | Raw damage of the direct hit on the collector and of each of the five blasts |
| `Napalm` | Raw damage of the direct hit on the collector and of the blast |
| `Gas` | Raw damage applied to each of the nine cells |
| `Veteran` | How many promotion steps each object takes |
| `Armor` | The armor multiplier |
| `Speed` | The speed multiplier |
| `Firepower` | The firepower multiplier |
| Every other result | Not read |

### Money and free units

Outside a campaign the money result pays a random figure between the third field and that figure plus 900 credits, inclusive. In a campaign it pays `SoloCrateMoney` instead, and a campaign whose `SoloCrateMoney` is `0` falls back to the random range. Campaign credits go to the house the local player commands whenever the collecting object belongs to a house the player drives, rather than to that collecting house.

The unit result picks a vehicle type in this order:

1. the `BaseUnit` type, when the MCV override above fired;
2. otherwise the first [`HarvesterUnit`](/keys/harvesterunit/) type, when the house owns a structure of the first [`BuildRefinery`](/keys/buildrefinery/) type and no vehicle of that first `HarvesterUnit` type;
3. [`UnitCrateType`](/keys/unitcratetype/), whenever it names a type;
4. otherwise a random UnitType, redrawn until one satisfies **all of**, in this order:

   - it is [`CrateGoodie=yes`](/keys/crategoodie/);
   - it is ownable by the collector's house;
   - **Any of:** bases are enabled for the match, or it is not the `BaseUnit` type.

:::caution[UnitCrateType cancels both rescues]
Step 3 overwrites whatever steps 1 and 2 chose, so naming a type there suppresses the free MCV given to a house that has lost its base and the free harvester given to a house that has lost its last one. Leaving `UnitCrateType=none` is what keeps both rescues reachable.
:::

The chosen vehicle is created for the collector's house in [limbo](/glossary/#limbo) and then taken out of it onto the crate cell, or onto a nearby cell its own speed type can occupy. Either success refuses the collector entry into the crate's cell, so the collector stops short of it. If neither placement succeeds the vehicle never leaves limbo: it is deleted and money is paid instead, and outside a campaign that figure is drawn from the `Unit` row's third field rather than the `Money` row's.

### Results that sweep a radius

`Cloak`, `Veteran`, `Armor`, `Speed`, and `Firepower` all sweep the ground layer and apply themselves to objects within [`CrateRadius`](/keys/crateradius/) of the center of the crate's cell. The cloak result marks each object as able to cloak, which [cloaking and detection](/systems/cloaking/) then acts on; the veterancy result steps each object up the [promotion ladder](/systems/veterancy/#promotion-without-kills) as many times as its third field says. None of the five tests ownership, so objects of every house inside the circle are affected alongside the collector's own.

The armor multiplier divides incoming damage while the speed and firepower multipliers multiply their quantities directly. All three store the third field as written: an armor value of `2` halves ordinary incoming damage, while `0.5` doubles it.

The armor sweep changes only an object whose armor multiplier is exactly `1`. An object an earlier armor crate has already boosted is therefore left as it is, and a collector whose armor multiplier has already changed can convert the whole result to money before this sweep runs.

:::caution[An armor value of zero leaves a zero divisor]
The armor value is not clamped. Ordinary positive damage divides by the stored multiplier, so do not set the `Armor` row's third field to `0`.
:::

### Results that reach the whole map

The heal result restores every object belonging to the collector's house to its maximum strength, wherever it stands. It applies no distance test at all, so `CrateRadius` has no bearing on it and the reach is the whole house rather than the crate's surroundings. [`HealCrateSound`](/keys/healcratesound/) plays at the collector when the collector belongs to the local player.

The darkness result reshrouds the whole map, and the reveal result reveals it, both only when the collecting house is the one controlled from the local machine. Repeated reveal results call the same reveal operation and leave an already revealed map revealed; darkness is the only crate result that restores shroud. A crate of either kind collected by a computer house is consumed and plays its animation without changing anyone's vision.

The missile result grants the collector's house a [one-time superweapon](/systems/superweapons/#one-shot-at-a-time), which is where the lookup that chooses it is covered. A house that already holds that weapon gains nothing at all, and the sidebar button appears only when the collector belongs to the local player.

### Damaging results

The explosion result damages the collector directly with the [`C4Warhead`](/keys/c4warhead/) warhead, then creates five blasts of the same damage and warhead scattered within two cells of the crate, each with its own explosion animation, lighting flash, and chain reaction.

The napalm result damages the collector with the [`FlameDamage`](/keys/flamedamage/) warhead and applies the same damage as a blast at the midpoint between the crate's cell and the collector, where it also creates an animation. That animation is fixed in the engine and picked by position rather than by name — it is whichever animation the rules register first — so nothing in `[Powerups]` reaches it.

The gas result applies its damage with the hard-coded `GAS` warhead to the crate's cell and to all eight neighbors, with no source recorded and chain reactions suppressed. Rules that register no warhead named `GAS` make the result do nothing rather than fail.

### Tiberium

The Tiberium result places one growth stage of a randomly drawn registered [Tiberium](/systems/tiberium/) type on the crate's cell, then between 10 and 20 more scattered within three cells of it. A draw landing on the Tiberium type at index 1 is redirected to the type at index 0, so index 1 never arrives by crate and its share of the draw falls to index 0 instead. A cell already holding that Tiberium gains a stage instead, and any other cell that cannot take it is left as it is.

## Settings and results without effect

Three result tokens have no result handler at all: `Invulnerability`, `IonStorm`, and `Pod`. Drawing one still consumes the crate and still plays that row's animation; nothing else happens. `Pod` shares its name with the [Drop Pods](/systems/drop-pods/) superweapon but has no connection to it.

`Squad` is rewritten to the money result immediately before the result runs, so its own handler is unreachable and its share behaves as extra weight on money. The payout in that case comes from the `Money` row's third field.

[`FreeMCV`](/keys/freemcv/) is parsed from `[CrateRules]` and never read again. The free-MCV override it names is applied unconditionally, so the setting cannot switch it on or off.

## What a crate leaves behind

The result animation named by the second `[Powerups]` field is created at the center of the crate's cell, raised slightly off the ground. It is chosen from the result that actually ran, so a converted crate plays the animation of what it was converted into — a wasted firepower crate plays the money animation. A unit result that successfully places its vehicle returns before this step, so a unit crate plays an animation only when the vehicle could not be created.

`Armor`, `Speed`, and `Firepower` each speak one EVA line per crate, and only when at least one affected object belongs to a locally controlled house.

An object whose armor or firepower multiplier is above 1, or an infantryman, vehicle or aircraft whose speed multiplier is, draws a different selection bracket. That bracket is the only lasting on-map sign of the three multiplier crates — a veterancy crate shows its promotion insignia and a cloak crate shows itself by cloaking — and it is not shown on every object: buildings and [`IsCoreDefender=yes`](/keys/iscoredefender/) vehicles draw a pip bar instead of a bracket and so never carry it.
