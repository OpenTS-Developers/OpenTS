---
title: Route search
summary: "Plans a route in blocks of cells, walks it cell by cell, and prices each step by what stands in the way."
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

An object told to go somewhere is handed a finished route before it sets off: a list of single steps from one [cell](/glossary/#cell) to the next, worked out in one go and then followed a step at a time. Nothing revises it while the object travels. A step that turns out to be blocked throws the whole list away and runs the search again.

Two questions are settled outside this page. Whether the destination is reachable at all is decided before any search is attempted, against [the zone map](/systems/movement-and-terrain/#the-zone-map). A destination in a different [movement zone](/glossary/#movement-zone) is refused without a search. Whether one particular cell may be stepped into is decided by [the per-step test](/systems/movement-and-terrain/#why-a-cell-refuses-a-vehicle). This page covers what is left: how the search chooses among the cells it is allowed to enter, and what that choice costs.

## How a route is found

The search runs in two stages. The first plans across the map in blocks of cells and produces a **corridor**: a chain of small blocks leading from the start to the destination. The second walks the map cell by cell and stays inside that corridor. Without the first stage, the second would have to weigh every cell between the two ends. The corridor is what makes a route across a large map affordable.

### Planning in blocks

The playfield is divided into blocks at three sizes: 8 by 8 cells, 4 by 4, and 2 by 2, each size cut along grid lines aligned to the map. A block at one size is a single run of connected cells sharing one [blockage rating](/systems/movement-and-terrain/#the-zone-map), no wider or taller than that size, with no abrupt height change across it. The block stage runs once at each size, largest first. The largest size answers to no parent, and each smaller size may enter only blocks lying inside one that the size above it settled on. The chain of 2-by-2 blocks that falls out of the last of the three is the corridor.

A block is entered only where the object's [movement zone](/keys/movementzone/) class accepts the rating of the ground it holds. That page owns which classes accept which. Where the block is accepted, the step into it is priced by that same rating, on a table the cell stage never uses:

| Rating of the block being entered | Price |
| --- | --- |
| Crushable, blocked, or partly blocked | 0 |
| Open land, water, or impassable | 1 |

The threat of the region the block sits in is added on top, through [`ThreatAvoidanceCoefficient`](/keys/threatavoidancecoefficient/). [Base defense response](/systems/base-attacked/#what-reads-the-map) owns where that per-region figure comes from. The coefficient is read from the type, and an [`AvoidThreats=yes`](/keys/avoidthreats/) override on its team's TeamType replaces it with `1`.

An ordinary vehicle's class accepts open land and nothing else, so every step costs it 1. Its corridor is then the chain that crosses the fewest blocks. For a class that accepts more than open land, some blocks cost nothing, and the corridor is drawn through them. A crushing vehicle's corridor runs through a run of sandbags rather than around it, because the wall costs nothing and the open ground beside it costs 1.

The corridor stage is skipped outright under **any of:**

- the object is a train, which [`IsTrain=yes`](/keys/istrain/) covers;
- the object has not yet entered the playable area, or is one of the few [allowed to leave the map](/keys/landable/);
- either end of the journey lies outside the playable area.

With the stage skipped, the cell search may spread across the whole playfield instead of staying inside a corridor.

:::caution[A corridor of more than 500 blocks is abandoned]
Each of the three block sizes has a list of 500 entries to record its chain in. A chain longer than the list is refused, and the cell search runs unrestricted instead.

The 2-by-2 chain is the one that fills first. A route crosses about one 2-by-2 block every two cells, so five hundred of them is a walk of roughly a thousand cells, half the 2,000-cell limit at the foot of this page. A block holds at most four cells of a route, and the estimate above rests on the usual two. A journey that long gives up the corridor and prices every cell it passes, which costs more effort than a corridor would have.

The block stage runs before the cell search, so this is settled before a single cell has been priced. It also runs on its own whenever the game measures how far an object would have to walk between two cells, and that measurement takes none of the skips above.
:::

### The cell-by-cell search

The cell stage starts at the cell the object is heading into, which is the cell it stands in when it is not moving, and spreads outward from there. Every cell it reaches holds two figures: what the steps taken to get there have cost, and that total plus a guess at what is left to run. The search repeatedly takes up whichever reached cell has the lowest second figure, prices the eight neighbors of that cell and, where the cell is a tunnel mouth, the cell at the tunnel's far end, then goes round again. It finishes when the cell it takes up is the destination.

The guess is the straight-line distance from the cell to the destination, measured in cells.

A cell outside the corridor is passed over, with two exceptions. A step onto a bridge deck is always considered, and so is a step into a cell with an object standing in any of the eight cells around it. That second exception is what lets a route work its way past an obstruction the corridor gave a wide berth.

### Effort, retries and failure

A pass of the cell stage may take up at most 65,527 cells, nine short of 65,536, and a pass that reaches that limit yields no route.

Where a pass fails and a corridor was in force, the block links the search could not walk are struck out, and both stages run again to a limit of five passes. Where the search stalled inside a block rather than at a link between two, the links around that block are struck out as well. Where that leaves no alternative chain at all, the corridor is abandoned, and the passes still to come search the map unrestricted.

A request that ends with no route at all arms the object's [`PathDelay`](/keys/pathdelay/) countdown. The object stands still until that expires, instead of searching again on the next frame.

The four rules settings the search reads, in the two sections that hold them:

```ini title="rules.ini"
[General]
CloseEnough=2         ; cells; a blocked object within this of its destination treats it as reached
Stray=3               ; cells; a team member may drift this far before it is ordered back

[AI]
PathDelay=.03         ; minutes an object stands still after a search that found nothing
BlockagePathDelay=45  ; frames it prefers to wait out a moving obstruction before insisting on routing around one
```

:::caution[A route completed on exactly the ten-thousandth cell is thrown away]
Alongside the effort limit, a finished pass is checked against a second figure. A pass that reaches the destination having taken up exactly 10,000 cells is treated as a failure. It hands back no route, although the route was found and is complete. No other count is treated that way. Nothing about such a route distinguishes it from one found a cell earlier or later.
:::

:::caution[A pass that reaches more than 131,072 cells stops taking new ones]
The effort limit counts the cells a pass takes up, but a cell is recorded the moment it is first priced, well before it is taken up, and for many cells it never is. Cells on the search's outer edge count against the record too. The record holds 131,072 of them. A pass that has filled it passes over every further cell it reaches, so it runs out of candidates and hands back no route.

A cell is recorded once and no more, so reaching that many needs ground to match. The pass must have more than 131,072 cells the object may enter. The playable area is a diamond rather than a square. One declared 256 cells on a side spans about 512 cells across the grid, so it holds about 131,000 cells where a square of the same declaration would hold 65,536. A cell spanned by a bridge is recorded twice over, once for the ground and once for the deck. The destination also has to be far enough off, or awkward enough to reach, for the search to spread over all that ground before it settles.
:::

## Why a route is not the shortest one

The search is built to return the cheapest route and does not, in general, return it. Two independent things inside it break that guarantee, and either one alone is enough.

**A cell is never reconsidered.** Once the search has reached a cell it never looks at it again, however cheaply it is reached later. Two tests early in each step do compare the cost of arriving through the cell in hand against the cost already recorded for the neighbor, and a cheaper arrival passes them. A third test, made just before the neighbor would be added, rejects any cell already reached and does not compare cost at all, so the cheaper arrival is discarded with the rest. Every cell keeps the first route into it that the search happened to find.

**The guess overshoots.** A diagonal step and a straight one have the same base price. Over clear ground, the least the remaining run can cost is the number of steps left, which is the larger of the two distances in cells. The guess used instead is the straight-line distance, which is larger than the true minimum for anything off a straight line, by about two fifths on an exact diagonal. A guess that comes in above the truth is what lets the search settle on a route while a cheaper one is still sitting unexamined.

The corridor adds a third cause from outside the search. The cell stage may not leave it, so a cheaper route through blocks the block stage passed over is never seen at all. The block stage chose those blocks on the separate table above, one on which a wall is free and the open ground beside it is not.

:::caution[A long way round is the ordinary output of the search]
None of the three is a threshold that can be crossed or a figure that can be moved. The threat term the block stage adds is the one key-driven part of the three, and it can only push a corridor away from a region, never shorten one. A route that takes a visibly longer way round while a shorter one stands clear is what the search does. It is not a sign that a terrain figure, a movement zone or a piece of map geometry is wrong.
:::

## What a step costs

[Path cost](/glossary/#path-cost) is the total the search adds up while it chooses between routes. Every entry below is one step's contribution to it, and the search works toward a cheap total rather than a guaranteed cheapest one.

A step is priced by why the cell being entered can be entered: the verdict the [per-step test](/systems/movement-and-terrain/#why-a-cell-refuses-a-vehicle) returns. Nothing about the ground itself reaches the price, so a road and a patch of rough ground cost the same. The land-type numbers act elsewhere, on the object's speed, which [movement and terrain](/systems/movement-and-terrain/#the-route-search-prices-no-terrain) sets out.

| Verdict on the cell being entered | Price |
| --- | --- |
| Clear | 1 |
| A closed friendly gate | 1 |
| Something moving through | 1, 4, or 1000 |
| A friendly object temporarily in the way | 8 |
| An enemy obstruction that could be destroyed | 20 |
| A friendly obstruction that could be destroyed | 60 |
| A cloaked enemy | 1000 |

A friendly obstruction that has to be shot through is priced at three times an enemy one, so a vehicle picks the enemy's wall over its own where both stand in reach. A closed friendly gate is priced like clear ground, so a route runs straight through the gates of its own base. A cell whose verdict is strictly prohibited is priced as well, at 10,000. No cell with that verdict is ever added to the search, and the destination is the one case where pricing a prohibited step ends the search rather than skipping it.

Something moving through is the one verdict with three prices. Where the request asks for no avoidance, the search follows the queue in front of the cell. It starts with the object standing there, then the cell that object is heading into, then whatever stands in that one, and follows that chain for up to ten objects. It prices the step at 1 where the queue ends in an empty cell, or in a stopped object with no route of its own. It prices the step at 4 where the queue instead reaches something that does not move under its own power, or runs the full ten deep. Where the request asks for avoidance, the queue is not followed at all: a merely preferred avoidance prices the step at 4 and an insisted-on one at 1000. [`BlockagePathDelay`](/keys/blockagepathdelay/) owns which of the three a retry asks for.

Three adjustments follow.

- The price is quadrupled where the cell has the mark collision avoidance puts up. That covers the cells the objects in the way are about to walk through, and the occupied cells immediately in front of the object being routed. The marks go up for the search and come down again after it, and only where avoidance was asked for.
- A sliver is added for the direction of the step. The four steps to an edge-sharing neighbor take `0.001` through `0.004` clockwise from north, and the four diagonals `0.005` through `0.008` clockwise from northeast. A tie is therefore settled in favor of stepping straight, and among steps of one kind in the order given above.
- A step through a tunnel is priced apart from all of this. It costs the larger of the two cell distances between the tunnel's mouths, along the cell grid's own axes, and takes neither the verdict price above nor the direction sliver.

## Where a route begins and ends

The destination searched for is not always the destination that was ordered. Where the cell ordered has a friendly object temporarily in the way and lies further off than [`CloseEnough`](/keys/closeenough/), a nearby enterable cell is looked for. For an object on a team, [`Stray`](/keys/stray/) sets that separation instead. The order moves to that cell under **all of:**

- one was found;
- it lies nearer the ordered cell than the object itself does;
- the walk from it to the ordered cell is no more than six cells longer than the straight distance between the two.

Where the cell ordered is strictly prohibited and holds a structure, the order moves to a nearby cell with none of those tests. Neither substitution is made for a train.

A destination spanned by a bridge has to be reached at the deck's height. Stopping at the ground beneath it does not count, and the search carries on.

Where the destination's own verdict is strictly prohibited, so that the step into it is never taken, the search stops as soon as it prices that step and hands back the route it has. That route ends beside the destination, at whichever cell the search happened to be taking up at that moment, not at the nearest or the best one. Two things fall out of that. A route consisting of nothing but the cell the object is already in is rejected, so an object standing beside a prohibited destination is left with no route at all. And a [`Passive=yes`](/keys/passive/) vehicle takes no such offer.

## Straightening the finished route

The move list is worked over twice before the object is given it, and both passes can change the ground the route covers.

The first looks for a corner where two diagonal runs meet at a right angle. It takes the steps either side of the corner, as many as the shorter of the two runs holds, and replaces them with a straight run twice that long, ending where they did. Any surplus steps of the longer run are left alone. The straight run is tested cell by cell, and is taken only where every cell of it is clear outright, not merely cheap, and is unmarked by claimed traffic. Where it does not fit, the attempt is shifted one step along and shortened by one, until it fits or is given up on. A corner involving a tunnel step is never touched.

The second looks only at the first twenty steps. It follows how far the route has carried the object from the point the current leg began, and where a step fails to carry it further, that wandering tail is thrown away. The tail is replotted as a two-leg run, one diagonal leg and one straight leg. Every cell of the replot must be clear outright and unmarked as before, and no more than three of them may be threatened. [`ThreatAvoidanceCoefficient`](/keys/threatavoidancecoefficient/) decides which cells count as threatened. The run is tried with the diagonal leg first and, if that fails, with the straight leg first; where neither order is clear the replot is abandoned and the original steps stand. A successful one can shorten the route. Nothing past the twentieth step is examined.

## Settings and state without effect

Three switches inside the search are set once, when the pathfinder is created, and are never changed. No rules key reaches them, no saved game holds them, and there is one pathfinder for the whole game.

- **Bridge avoidance** is off. The step cost has a branch that would multiply a step onto a bridge by ten where the span does not continue and by two where it does, so that routes shy away from bridges. The branch never runs, so a bridge step is priced exactly like a ground step.
- **The cost multiplier** is fixed at `1`. Every step's price passes through it before the direction sliver is added, and it changes nothing. It is the figure that would let what stands in the way weigh more heavily against distance.
- **The locomotor question** is on, and this one leaves nothing unreachable. A vehicle's per-step test ends by putting the cell to the object's own travel routine. The switch decides whether it bothers, and the search always has it ask. [Nine of the ten locomotors accept every cell](/systems/movement-and-terrain/#why-a-cell-refuses-a-vehicle) and the tunneling one does not. What is settled here, then, is that a tunneling vehicle is held to the burrowing test while its route is being plotted as well as while it drives. Other parts of the game outside movement take the cell without asking.

Two limits the search is handed are not read. The one routine that asks for a route names a ceiling on the list it will accept, and names a clear cell as the worst verdict it is prepared to walk into. Neither limit reaches the search, which sizes the route by what it finds and prices worse verdicts rather than refusing them. The search also records the type's [`Speed=`](/keys/speed/) as it starts and reads it nowhere.

:::caution[A route longer than 2,000 cells is never returned]
The search stops extending a route at 2,000 cells, so a journey that needs a longer one ends with no route at all. That arms [`PathDelay`](/keys/pathdelay/) like any other failure, and the object stands still until the countdown expires.

A single pass may still take up more than thirty times that many cells before it gives up. The limit is on the route returned, not on the effort spent looking for one. The buffer the caller hands the search is sized one entry longer than the longest route the search can return, and the ceiling the caller names is that same figure, which the search never reads. No rules key moves any of the limits on this page.
:::
