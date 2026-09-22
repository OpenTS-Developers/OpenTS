---
title: Route search
summary: "Plans a route across blocks of cells, then cell by cell, and prices each step by what stands in the way."
category: units-movement
keys:
  - AvoidThreats
  - BlockagePathDelay
  - CloseEnough
  - IsTrain
  - Landable
  - MovementZone
  - Passive
  - PathDelay
  - Speed
  - Stray
  - ThreatAvoidanceCoefficient
related:
  - type: system
    id: movement-and-terrain
  - type: system
    id: base-attacked
  - type: enum
    id: MZoneType
  - type: internal
    id: locomotion
---

A vehicle or infantryman ordered to move is given a route: a list of single steps from one [cell](/glossary/#cell) to the next. The search plans the whole way to the destination, but the object keeps only the next 24 steps. When it has taken them, a new search runs from where it stands. A step that turns out to be blocked can also start a new search.

Two decisions belong to [movement and terrain](/systems/movement-and-terrain/). [The zone map](/systems/movement-and-terrain/#the-zone-map) decides whether the destination can be reached at all, and an order to a destination in a different [movement zone](/glossary/#movement-zone) is refused before any search runs. [The per-step test](/systems/movement-and-terrain/#why-a-cell-refuses-a-vehicle) decides whether one cell may be entered. This page covers how the search chooses among the cells it may enter, and what each choice costs.

## How a route is found

The search runs in two stages. The block stage plans across the map in blocks of cells and produces a **corridor**: a chain of small blocks leading from the start to the destination. The cell stage then searches cell by cell and stays inside that corridor. The corridor is what keeps a route across a large map affordable, because the cell stage never has to weigh every cell between the two ends.

### Planning in blocks

The playfield is divided into squares at three sizes: 8 by 8 cells, 4 by 4, and 2 by 2, each cut along grid lines aligned to the map. A block is one connected group of cells inside one square. Its cells share one [blockage rating](/systems/movement-and-terrain/#the-zone-map), and no two neighboring cells in it differ by two or more height levels. One square can therefore hold several blocks.

The block stage runs once at each size, largest first. At the 4-by-4 and 2-by-2 sizes it may enter only blocks that lie inside the chain found at the size above. A crushable block is the exception and may be entered outside that chain. The chain of 2-by-2 blocks found last is the corridor.

A block can be entered only where the object's [movement zone](/keys/movementzone/) accepts its rating. That page owns which movement zones accept which ratings. The step into an accepted block is priced by its rating, on a table the cell stage never uses:

| Rating of the block being entered | Price |
| --- | --- |
| Crushable, blocked, or partly blocked | 0 |
| Open land, water, or impassable | 1 |

An ordinary vehicle's movement zone accepts open land and nothing else, so every block costs it 1, and its corridor is the chain that crosses the fewest blocks. A movement zone that accepts more than open land finds some blocks free, and its corridor is drawn through them. A crushing vehicle's corridor runs through a line of crushable wall instead of around it, because the wall costs nothing and the open ground beside it costs 1.

At the 8-by-8 and 4-by-4 sizes, the threat of the ground the step crosses is added to the price. The threat is multiplied by [`ThreatAvoidanceCoefficient`](/keys/threatavoidancecoefficient/) and rounded down to a whole number, so a product below 1 adds nothing. The 2-by-2 stage adds no threat. [Base defense response](/systems/base-attacked/#what-reads-the-map) explains where each region's threat comes from. The coefficient comes from the object's type. A team whose TeamType sets [`AvoidThreats=yes`](/keys/avoidthreats/) uses `1` for its members instead.

The block stage is skipped under **any of:**

- the object is a train ([`IsTrain=yes`](/keys/istrain/));
- the object has not yet entered the playable area, or is one of the few objects [allowed to leave the map](/keys/landable/);
- either end of the journey lies outside the playable area.

With no corridor, the cell stage may spread across the whole map.

:::caution[Very long corridors are abandoned]
The block stage records each chain in a list of 500 blocks. Before the first pass, a chain longer than that is refused, and the cell stage runs without a corridor. It may then spread across the whole map, which costs far more effort. After a failed pass, the same refusal ends the request with no route.

The 2-by-2 chain is the longest, so it reaches the limit first. A route crosses a 2-by-2 block about every two cells, so the limit is reached on journeys of roughly 1,000 cells, and sooner where the ground breaks into smaller blocks.

The game also uses the block stage alone to estimate walking distance, for example when it picks a nearby cell to stand in for an occupied destination. That estimate reports a destination whose corridor exceeds 500 blocks as unreachable.
:::

### The cell-by-cell search

The cell stage starts from the cell the object is heading into, or the cell it stands in when it is not moving. Each cell the search reaches carries two figures: the cost of the steps taken to get there, and that cost plus an estimate of what remains. The estimate is the straight-line distance to the destination, measured in cells.

The search repeatedly takes up the reached cell with the lowest second figure. It prices the step into each of that cell's eight neighbors and, at a tunnel mouth, the step to the tunnel's far end. It finishes when the cell it takes up is the destination.

A cell outside the corridor is skipped, with two exceptions:

- a cell on a bridge deck;
- a cell next to a vehicle, infantryman, aircraft, wall or terrain object such as a tree, in any of the eight directions;
- a cell on or beside a structure.

The last two exceptions let a route step out of the corridor to get around an object standing in it.

### Effort, retries and failure

One pass of the cell stage may take up at most 65,527 cells. A pass that reaches that limit finds no route.

When a pass fails inside a corridor, the search strikes out corridor links at the furthest corridor block it reached, then runs both stages again. A request gets at most five passes. Depending on where the search stopped, it strikes one of two sets of links:

- the links out of that block that the search could not use;
- the corridor links around that block.

In the second case, if that block is not on the corridor or the corridor has only one block, the corridor is dropped instead. If the five passes are not used up, one final pass then searches without it.

If the block stage then finds no chain that avoids the struck links, the request ends with no route.

When the search finds no route, the object stops. What happens next depends on where the destination is:

- **More than one cell away, or under a bridge while the object is not on a bridge:** the object gives up the order. It leaves its team and drops its destination and target. A player's object takes the guard mission. A computer-controlled object takes the area guard mission. Outside a campaign it is also sent to a random cell around its base, the same kind of place new units are sent to wait.
- **Within one cell:** the object keeps its destination, and the [`PathDelay`](/keys/pathdelay/) countdown holds off its next search.

These four settings affect the search. All are in `rules.ini`; the values shown are examples, and the linked key pages give the defaults.

```ini title="rules.ini"
[General]
CloseEnough=2         ; cells; an occupied destination further away than this is swapped for a nearby cell
Stray=3               ; cells; the same distance for an object on a team

[AI]
PathDelay=.03         ; minutes an object waits before it searches again after a failed search
BlockagePathDelay=45  ; frames a blocked object prefers to wait before it insists on a detour
```

:::caution[A route found on exactly the 10,000th cell is discarded]
A pass that reaches the destination after taking up exactly 10,000 cells is treated as a failure and returns no route, although the route is complete. No other count is treated this way.
:::

:::caution[A pass that records 131,072 cells stops adding new ones]
The search records a cell when it first prices the step into it, not when it takes the cell up. The record holds 131,072 cells. Once it is full, the pass stops recording new cells but keeps taking up the ones already recorded. It then fails unless the destination was recorded before the record filled.

Each cell is recorded only once, and a cell spanned by a bridge counts twice, once for the ground and once for the deck. A pass takes up at most 65,527 cells, so the record fills only when more than about 65,500 recorded cells are waiting to be taken up at the same moment.
:::

## Why a route is not the shortest one

The search aims for the cheapest route but does not always return it. Two properties of the search cause this, and either one is enough.

**A cell keeps the first route into it.** Once the search has reached a cell, it never replaces the route into that cell, even when it later finds a cheaper one.

**The estimate runs high.** A diagonal step and a straight step have the same base price. Over clear ground, the fewest steps left to the destination is the larger of the two cell distances. The estimate uses the straight-line distance, which is larger for any destination not due along a row or column. On an exact diagonal it is about two fifths larger. An estimate above the true remaining cost lets the search settle on a route while a cheaper one is still unexamined.

The corridor adds a third cause. The cell stage cannot leave it, so it never sees a cheaper route through blocks the block stage passed over. The block stage chose its blocks on a different price table, where a crushable or blocked block costs nothing to a movement zone that accepts it.

:::caution[A longer way round is normal]
No setting removes any of the three causes. Threat avoidance is the only part a key controls, and it can only push a corridor away from a region. A route that takes a visibly longer way round while a shorter one is clear does not mean that a terrain figure, a movement zone or the map is wrong.
:::

## What a step costs

[Path cost](/glossary/#path-cost) is the total the search adds up to compare routes. Each entry below is one step's contribution to it.

A step's price depends on the verdict the [per-step test](/systems/movement-and-terrain/#why-a-cell-refuses-a-vehicle) gives for the cell being entered. The ground itself does not affect the price, so a road and rough ground cost the same. Land-type figures affect the object's speed instead, as [movement and terrain](/systems/movement-and-terrain/#the-route-search-prices-no-terrain) explains.

| Verdict on the cell being entered | Price |
| --- | --- |
| Clear | 1 |
| A closed friendly gate | 1 |
| Something moving through | 1, 4, or 1000 |
| A friendly object standing in the way | 8 |
| An enemy obstruction that could be destroyed | 20 |
| A friendly obstruction that could be destroyed | 60 |
| A cloaked enemy | 1000 |

A friendly obstruction costs three times as much as an enemy one, so a vehicle that can reach both would rather shoot through the enemy's wall than its own. A closed friendly gate costs the same as clear ground, so a route runs straight through its house's gates and its allies' gates.

A strictly prohibited cell is never entered. When the prohibited cell is the destination, the search may stop beside it, as [where a route begins and ends](#where-a-route-begins-and-ends) describes.

In the search, a train treats every verdict except strictly prohibited as clear, so each step it can take costs 1.

### Something moving through

The price for a cell that something is moving through depends on whether the search avoids moving objects. An ordinary search asks for no avoidance. When a step is refused because something is moving through the next cell, the new search prefers avoidance until the object's [`BlockagePathDelay`](/keys/blockagepathdelay/) countdown runs out, and insists on it after that.

- **Avoidance preferred:** 4.
- **Avoidance insisted on:** 1000.
- **No avoidance:** the search follows the line of objects ahead of the cell, up to ten deep. It starts with the object in the cell, then looks at the cell that object is heading into, and so on. The price is 1 if the line ends in an empty cell or in a stopped object with no route of its own. The price is 4 in three cases: the line reaches something that cannot move under its own power, it reaches a stopped object about to enter a tunnel, or it runs ten deep.

The cell ahead is the one the routed object faces. A preferred avoidance applies only when a qualifying object stands in the cell ahead or is heading into it. Otherwise the search runs as if no avoidance had been asked for.

A qualifying object is a vehicle or infantryman of a different type, slower by [`Speed`](/keys/speed/), with at least two steps of its route left, or three for infantry.

An insisted-on avoidance always applies. It counts any vehicle or infantryman in or heading into the cell ahead that has those steps left, whatever its type or speed.

While avoidance applies, a step into a marked cell costs four times its price. The search marks:

- the rest of the route of each object it counts;
- occupied cells within two cells of the cell ahead, other than the routed object's cell;
- the cell ahead itself.

Each mark flips the cell, so a cell marked twice ends up unmarked. An occupied cell ahead is therefore left unmarked. So is a cell on two of the marked routes, or an occupied cell near the cell ahead that is also on one.

The marks exist only for the duration of the search.

### Direction and tunnels

Each step also adds a tiny amount for its direction, which settles ties between otherwise equal routes. Steps to the four edge-sharing neighbors add `0.001` to `0.004`, clockwise from north. The four diagonal steps add `0.005` to `0.008`, clockwise from northeast. A tie therefore goes to stepping straight, and among steps of the same kind, to the direction listed first.

A step through a tunnel costs the larger of the two cell distances between its mouths, along the cell grid's axes. It takes neither a verdict price nor a direction amount.

## Where a route begins and ends

The search does not always look for the ordered cell. If a friendly object stands in the ordered cell and the ordered cell is further away than [`CloseEnough`](/keys/closeenough/), the game looks for a nearby cell the object can enter. For an object on a team, [`Stray`](/keys/stray/) replaces `CloseEnough`. The order moves to that cell under **all of:**

- a cell was found;
- it is nearer the ordered cell than the object is;
- the estimated walk from it to the ordered cell is at most six cells longer than the straight distance between the two.

If the ordered cell is strictly prohibited and holds a structure, the order moves to a nearby enterable cell without these tests. Neither substitution is made for a train.

A train's route never enters the three cells that are behind the train when the search starts.

A destination spanned by a bridge must be reached at deck height. Reaching the ground beneath the bridge does not end the search.

If the destination is strictly prohibited, the search stops when it prices the step into the destination from a cell within one height level of it. The route then ends at that cell, beside the destination. It is the first such cell the search takes up, not necessarily the nearest or best one. Two exceptions apply:

- An object already standing next to such a destination gets no route at all, because a route that never leaves the object's cell is rejected.
- A [`Passive=yes`](/keys/passive/) vehicle does not stop beside the destination. It keeps searching, and it gets no route.

## Straightening the finished route

The finished route goes through two straightening passes before the object gets it. Both can change the ground the route covers.

The first pass looks for a corner where two diagonal runs meet at a right angle, such as northeast steps followed by southeast steps. Take the shorter of the two runs and call its length *n*. The pass replaces the last *n* steps before the corner and the first *n* after it with 2*n* straight steps that end at the same cell. The replacement is used only where every cell of it is clear, not merely cheap, and not marked by [avoidance](#something-moving-through). It is also refused when the threat at the cell it starts from, multiplied by [`ThreatAvoidanceCoefficient`](/keys/threatavoidancecoefficient/), is `1` or more. If the replacement does not fit, the pass starts it one step later and makes it one step shorter each side, until it fits or runs out. A corner that involves a tunnel step is never changed.

The second pass examines only the first 20 steps, and follows them in stretches between turns. When a step leaves the object no further from the stretch's first cell than the step before did, the pass replots the stretch as a diagonal run and a straight run. It tries the diagonal run first, then the straight run first. If neither order fits, the original steps stand. A successful replot can shorten the route.

Every cell of a replot must be clear and unmarked. A threatened cell also blocks a replot within the 20 steps. A final replot at the end of them may cross up to three threatened cells. [`ThreatAvoidanceCoefficient`](/keys/threatavoidancecoefficient/) decides which cells count as threatened.

## Settings and state without effect

The search has three fixed switches. No rules key reaches them and no saved game stores them.

- **Bridge avoidance is off.** A step onto a bridge deck costs the same as a step on the ground.
- **Step prices are not scaled.** No setting changes how heavily step prices count against the distance estimate.
- **The locomotor is always consulted.** The [tunnel locomotor refuses cells a subterranean vehicle may not burrow through](/systems/movement-and-terrain/#why-a-cell-refuses-a-vehicle), and the other nine accept every cell. A tunneling vehicle is therefore held to the burrowing test while its route is planned, as well as while it drives.

:::caution[Routes longer than 2,000 steps are never returned]
The search stops extending a route at 1,999 steps, so a journey that needs 2,000 or more gets none, with the result described under [effort, retries and failure](#effort-retries-and-failure). A pass can still take up more than thirty times that many cells before it gives up, because this limit applies to the route returned, not to the search effort. No rules key changes any of the limits on this page.
:::
