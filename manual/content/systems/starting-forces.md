---
title: Starting forces
summary: "Places each house's base unit and a budget of randomly chosen vehicles and infantry around its start position as a skirmish or multiplayer match begins."
category: maps-scenarios
keys:
  - AllowedToStartInMultiplayer
  - BaseUnit
  - Bases
  - Cost
  - InitialVeteran
  - MultiplayPassive
  - Owner
  - TechLevel
  - UnitCount
related:
  - type: key
    id: Official
    scope: scenarios-2
  - type: format
    id: spawn-ini
  - type: system
    id: crates
  - type: system
    id: observers
  - type: system
    id: veterancy
---

Each house in a skirmish or multiplayer match starts with a base unit when bases are on, followed by randomly chosen vehicles and infantry up to a budget. The forces are placed once, as the scenario finishes loading. A campaign mission gets them only when it is a [generated map](/systems/map-generation/).

A house whose country sets [`MultiplayPassive`](/keys/multiplaypassive/) gets no starting forces and holds no start position. Neither does an [observer's](/systems/observers/) house.

A mod sets the starting forces in `rules.ini`:

```ini title="rules.ini"
; MYINF and MYCIV must also be listed in [InfantryTypes]

[General]
BaseUnit=MCV    ; the first entry a house may own becomes its starting base unit

[MultiplayerDefaults]
UnitCount=8     ; the setup screen's unit count starts here

[MYINF]         ; example InfantryType that joins the draw
Cost=100        ; what a drawn type charges the budget
TechLevel=2     ; the house's tech level must reach this
Owner=GDI       ; the house's country must be listed here

[MYCIV]         ; example InfantryType held out of the draw
AllowedToStartInMultiplayer=no
```

In this example, a `GDI` house whose tech level is at least `2` can draw `MYINF`. No house draws `MYCIV`, and its price does not count toward the budget.

## The budget

Every house spends the same budget: the average price of the allowed types, multiplied by the match's unit count.

The average covers every InfantryType and every UnitType set to [`AllowedToStartInMultiplayer=yes`](/keys/allowedtostartinmultiplayer/), except the UnitTypes listed in [`BaseUnit`](/keys/baseunit/). Each type counts once at its [`Cost`](/keys/cost/#scope-aircrafttype), whatever its tech level and whichever countries may own it.

The unit count is a match option, set by `UnitCount` in the [launch file's `[Settings]`](/formats/spawn-ini/#the-options-every-house-plays-under) or on the setup screen. [`UnitCount`](/keys/unitcount/) in `[MultiplayerDefaults]` gives the setup screen its starting figure.

With [bases](/keys/bases/) on and `BaseUnit` naming at least one type, the unit count is reduced by one before the multiplication. That leaves room in the budget for the base unit, which is never charged against it.

If no type is allowed, the average and the budget are both zero. Each house then starts with its base unit alone, or with nothing when bases are off.

## Each house's shortlist

Each house draws only from its shortlist of allowed types. A type is on the shortlist when both of these hold:

- its [`TechLevel`](/keys/techlevel/#scope-aircrafttype) is at or below the house's tech level;
- its [`Owner`](/keys/owner/) lists the house's country.

Vehicles and infantry have separate shortlists. `BaseUnit` types never appear on the vehicle shortlist.

## The start position

Each playing house starts at one of the map's placed waypoints `0` through `7`. A house left over when those run out starts on open ground.

Every placed waypoint from `0` to `7` is eligible, with one exception. On a map whose [`Official`](/keys/official/#scope-scenarios-2) flag is set, only waypoints below a cutoff are eligible; the key page gives the cutoff. The exception does not apply when any playing seat in the launch file names a start position.

Houses take positions in this order:

1. A house whose seat names a start position in [the launch file](/formats/spawn-ini/#who-is-playing) holds it before anybody draws. If that waypoint is not eligible, or another house already holds it, the house draws with the others instead.
2. The remaining houses draw in turn. When no position is held yet, the house takes an eligible position at random. Otherwise it takes the open position with the greatest total distance from the positions already held.
3. A house still without a position once every eligible waypoint is held starts at a random spot where an 8-by-8 block of cells is open ground that a tracked vehicle can cross. This cell is found after the map has loaded, and the house holds no numbered position.

On a map read from a file, the numbered positions are settled after the waypoints are read and before the spawn house sections, teams, triggers, and objects. That lets a [spawn house](/formats/scenario-objects/#spawn-houses) name the house that starts at each position.

## The base unit

With bases on, each house starts with the first [`BaseUnit`](/keys/baseunit/) entry that the country the house [acts as](/keys/actslike/) may own. If that country may own none of them, the house gets the first entry. With bases off, or with `BaseUnit` empty, no base unit is placed.

The base unit goes on the start cell when it can stand there. Otherwise the [placement search](#where-an-object-lands) puts it on the nearest cell it finds, from one to thirty-one cells out. If no cell takes it, the house has no base unit.

With `AutoDeployMCV=yes` in the [launch file's `[Settings]`](/formats/spawn-ini/#the-options-every-house-plays-under), every house's base unit starts deploying as soon as it is placed. This includes computer houses. The unit deploys where it stands, with the building's foundation laid over and around the unit's cell.

If the ground there cannot take the building, or an object stands in its foundation, the unit stays undeployed. A human player's base unit waits where it was placed. A computer house's base unit drives off to find ground it can deploy on, provided its building is listed in [`BuildConst`](/keys/buildconst/); otherwise it hunts enemies.

## Spending the budget

Each house draws one object at a time and adds each placed object's `Cost` to what it has spent. Drawing stops once the spent total reaches the budget, so the last object can take the total past it.

Each draw picks, in this order:

1. While less than two thirds of the budget is spent and the vehicle shortlist is not empty, a random vehicle from it.
2. Otherwise, a random infantry type from the infantry shortlist.
3. If step 2 finds the infantry shortlist empty, the house stops drawing. [When placement fails](#when-placement-fails) covers the result.

If the [placement search](#where-an-object-lands) cannot place a drawn object, the object is removed without being charged, and the house draws again.

A placed object starts on guard, or on area guard for a computer house. With [`InitialVeteran`](/keys/initialveteran/) set, each placed object starts [elite](/systems/veterancy/). Neither applies to the base unit.

## Where an object lands

The search first tries the start cell, then works outward one distance at a time. The random objects are tried from three to thirty-two cells out, and the base unit from one to thirty-one cells out. With bases off, the first random object that can stand on the start cell is placed there; with bases on, the base unit holds it whenever the base unit could stand there.

The random objects start three cells out to leave the base unit room to deploy. The shifted second pass described below can still place a random object two cells from the start, and near a map corner an object can land closer still.

At each distance, the search starts from a random compass direction and tries the cell that far out in each of the eight directions in turn. If none takes the object, it tries the same eight cells again, each shifted at random by up to one cell along each axis. A candidate that lands back on the start cell is skipped.

A candidate that falls off the map is skipped. Near one of the map's four corners, a candidate past the corner can instead be pulled back onto it, so objects bunch there.

A cell that already holds a vehicle, infantry, aircraft, or structure is refused, except that infantry may share a cell with infantry. Otherwise the object's placement test decides, so a cell the type cannot enter is refused too.

## When placement fails

:::danger[Leave room around each start position for the whole budget]
The house draws again after every object the search cannot place, with no limit on attempts. If the search area around a start position cannot hold what the budget buys, the house keeps drawing and discarding objects, and the scenario never finishes loading.
:::

:::caution[An empty shortlist leaves a house short of its budget]
Once two thirds of the budget is spent, only infantry is drawn. A house with no infantry on its shortlist keeps the vehicles it has drawn by then and gets nothing more.

A house with nothing on either shortlist gets no random objects. This happens when every allowed type is above the house's tech level or not ownable by its country. Denying every type makes every budget zero, with the same result.
:::
