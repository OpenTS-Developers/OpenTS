---
title: Route search
summary: "Plans a route in blocks of cells, walks it cell by cell, and prices each step by what stands in the way rather than by the ground it crosses."
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

Two questions are settled outside this page. Whether the destination is reachable at all is decided before any search is attempted, against [the zone map](/systems/movement-and-terrain/#the-zone-map), and a destination in a different [movement zone](/glossary/#movement-zone) is refused without a search. Whether one particular cell may be stepped into is decided by [the per-step test](/systems/movement-and-terrain/#why-a-cell-refuses-a-vehicle). What is left is the subject here: how the search chooses among the cells it is allowed to enter, and what that choice costs.

## How a route is found

The search runs in two stages. The first plans across the map in blocks of cells and produces a **corridor** — a chain of small blocks leading from the start to the destination. The second walks the map cell by cell and is confined to that corridor. Without the first stage the second would have to weigh every cell between the two ends, and the first is what makes a route across a large map affordable.

### Planning in blocks

The playfield is divided into blocks three times over — 8 by 8 cells, 4 by 4 and 2 by 2 — and each block holds one run of connected ground of a single kind with no abrupt height change across it. The block stage runs once at each size, largest first, and every pass may enter only those blocks whose larger parent lay on the route the pass before it settled on. The chain of 2-by-2 blocks that falls out of the last pass is the corridor.

A block is entered only where the object's [movement zone](/keys/movementzone/) class accepts the [blockage rating](/systems/movement-and-terrain/#the-zone-map) of the ground it holds, and that page owns which classes accept which. Where the block is accepted, the step into it is priced by that same rating, on a table the cell stage never uses:

| Rating of the block being entered | Price |
| --- | --- |
| Crushable, blocked, or partly blocked | 0 |
| Open land, water, or impassable | 1 |

The threat of the region the block sits in is added on top, through [`ThreatAvoidanceCoefficient`](/keys/threatavoidancecoefficient/), which owns that term and the [`AvoidThreats=yes`](/keys/avoidthreats/) override that is normally the only thing to raise it above zero.

For an ordinary vehicle, whose class accepts open land and nothing else, every step costs 1 and the corridor is simply the chain crossing the fewest blocks. For a class that accepts more, the free ratings tilt it: a crushing vehicle's corridor is drawn through a run of sandbags rather than around it, because the wall costs nothing and the open ground beside it costs 1.

The corridor stage is skipped outright under **any of:**

- the object is a train, which [`IsTrain=yes`](/keys/istrain/) covers;
- the object has not yet entered the playable area, or is one of the few [allowed to leave the map](/keys/landable/);
- either end of the journey lies outside the playable area.

With the stage skipped the cell search is free to spread anywhere on the playfield.

:::caution[A corridor of more than 500 blocks is abandoned]
Each of the three block sizes has a list of 500 entries to record its chain in. A chain longer than the list is refused, and the cell search runs unrestricted instead.

The 2-by-2 chain is the long one. A route crosses about one 2-by-2 block every two cells, and four cells of route is the most that can ever fall inside a single block, so 500 of them is a walk of roughly a thousand cells -- half of what it takes to fill the move list at the foot of this page. A journey that long gives up the corridor and prices every cell it passes, which costs more effort than a corridor would have.

The block stage runs before the cell search, so this is settled before a single cell has been priced. It also runs on its own whenever the game measures how far an object would have to walk between two cells, and that measurement takes none of the skips above.
:::

### The cell-by-cell search

The cell stage starts at the cell the object is heading into — the cell it stands in, when it is not moving — and spreads outward. Every cell it reaches carries two figures: what the steps taken to get there have cost, and that total plus a guess at what is left to run. The search repeatedly takes up whichever reached cell has the lowest second figure, prices its eight neighbors and the far mouth of a tunnel where the cell holds one, and goes round again. It finishes when the cell it takes up is the destination.

The guess is the straight-line distance from the cell to the destination, measured in cells.

A cell outside the corridor is passed over, with two exceptions: one reached up on a bridge deck is always considered, and so is one with an object standing in any of the eight cells around it — which is what lets a route work its way past an obstruction the corridor gave a wide berth.

### Effort, retries and failure

One pass of the cell stage may take up at most 65,527 cells, and a pass that reaches that limit yields no route.

Where a pass fails and a corridor was in force, the block links the search choked on are struck out and both stages run again, to a limit of five passes. Where the search stalled inside a block rather than at a link between two, the links around it are struck out as well; and where that leaves no alternative chain at all, the corridor is abandoned and the passes still to come search the map unrestricted.

A request that ends with no route at all arms the object's [`PathDelay`](/keys/pathdelay/) countdown, and it stands still until that expires rather than searching again on the next frame.

:::caution[A route completed on exactly the ten-thousandth cell is thrown away]
Alongside the effort limit, a finished pass is checked against a second figure. A pass that reaches the destination having taken up exactly 10,000 cells is treated as a failure and hands back no route, although the route was found and is complete. No other count is treated that way, and nothing about such a route distinguishes it from one found a cell earlier or later.
:::

:::caution[A pass that reaches more than 131,072 cells stops taking new ones]
The effort limit counts the cells a pass takes up, but a cell is recorded the moment it is first priced -- well before it is taken up, and for many cells that never are. The cells on the search's outer edge count against the record too. The record holds 131,072 of them. A pass that has filled it passes over every further cell it reaches, so it runs out of candidates and hands back no route.

A cell is recorded once and no more, so reaching that many needs ground to match: more than 131,072 cells the object may enter in a single pass, which a square playable area of open ground passes at around 256 cells on a side. A cell spanned by a bridge is recorded twice over, once for the ground and once for the deck. The destination has also to be far enough off, or awkward enough to arrive at, that the search spreads over all of that ground before it settles on a route.
:::

## Why a route is not the shortest one

The search is built to return the cheapest route and does not, in general, return it. Two independent things inside it break that guarantee, and either one alone is enough.

**A cell is never reconsidered.** Once the search has reached a cell it never looks at it again, however cheaply it is reached later. Two tests early in each step do compare the cost of arriving through the cell in hand against the cost already recorded for the neighbor, and they let a cheaper arrival through — but a third test, made just before the neighbor would be added, rejects any cell already reached and does not consult cost at all. The cheaper arrival is discarded with the rest, and every cell keeps the first route into it that the search happened to find.

**The guess overshoots.** A diagonal step is priced exactly like a straight one, so the least the remaining run can possibly cost, over clear ground, is the number of steps left — the larger of the two distances in cells. The guess used instead is the straight-line distance, which is larger than that for anything off a straight line, by up to two fifths on an exact diagonal. A guess that comes in above the truth is what lets the search settle on a route while a cheaper one is still sitting unexamined.

The corridor adds a third cause from outside the search. The cell stage may not leave it, so a cheaper route through blocks the block stage passed over is never seen at all — and the block stage chose those blocks on the separate table above, one on which a wall is free and the open ground beside it is not.

:::caution[A long way round is the ordinary output of the search]
None of the three is a threshold that can be crossed or a figure that can be moved. A route that takes a visibly longer way round while a shorter one stands clear is what the search does, not a sign that a terrain figure, a movement zone or a piece of map geometry is wrong.
:::

## What a step costs

A step is priced by why the cell being entered can be entered — the verdict the per-step test returns — and by nothing about the ground itself. A road and a patch of rough ground cost it the same; the [path cost](/glossary/#path-cost) entry owns why the terrain figures do not reach it.

| Verdict on the cell being entered | Price |
| --- | --- |
| Clear | 1 |
| A closed friendly gate | 1 |
| Something moving through | 1 or 4 |
| A friendly object temporarily in the way | 8 |
| An enemy obstruction that could be destroyed | 20 |
| A friendly obstruction that could be destroyed | 60 |
| A cloaked enemy | 1000 |

A friendly obstruction that has to be shot through is priced at three times an enemy one, so a vehicle picks the enemy's wall over its own where both stand in reach. A closed friendly gate is priced like clear ground, so a route runs straight through the gates of its own base. A cell whose verdict is strictly prohibited is priced as well, at 10,000, and the price is never paid: such a cell is never added to the search.

Something moving through is the one verdict costed twice over. Where the request asks for no avoidance, the search follows the queue in front of the cell — the object standing there, then the cell that object is heading into, then whatever stands in that one, for up to ten objects — and prices the step at 1 where the queue ends in an empty cell or in a stopped object with no route of its own. It prices the step at 4 where the queue instead reaches something that does not move under its own power, or runs the full ten deep. Where the request asks for avoidance the queue is not followed at all: a merely preferred avoidance prices the step at 4 and an insisted-on one at 1000, and [`BlockagePathDelay`](/keys/blockagepathdelay/) owns which of the three a retry asks for.

Three adjustments follow.

- The price is quadrupled where the cell carries the mark collision avoidance puts up, which covers both the cells the objects in the way are about to walk through and the occupied cells immediately in front of the object being routed. The marks go up for the search and come down again after it, and only where avoidance was asked for.
- A sliver is added for the direction of the step, between `0.001` and `0.008`. The four steps to an edge-sharing neighbor take the four smallest and the four diagonals the four largest, so a tie is settled in favor of stepping straight, and among steps of one kind in a fixed order that never varies.
- A step through a tunnel is priced apart from all of this. It costs the larger of the two distances between the tunnel's mouths, in cells, and takes neither the direction sliver nor anything else above.

## Where a route begins and ends

The destination searched for is not always the destination that was ordered. Where the cell ordered has a friendly object temporarily in the way and lies further off than [`CloseEnough`](/keys/closeenough/) — or [`Stray`](/keys/stray/), for an object on a team — a nearby enterable cell is looked for, and the order moves to it under **all of:**

- one was found;
- it lies nearer the ordered cell than the object itself does;
- the walk from it to the ordered cell is no more than six cells longer than the straight distance between the two.

Where the cell ordered is strictly prohibited and holds a structure, the order moves to a nearby cell with none of those tests. Neither substitution is made for a train.

A destination spanned by a bridge has to be reached at the deck's height. Arriving at the ground beneath it is not arriving, and the search carries on.

Where the search touches the destination and finds it strictly prohibited, it stops there and hands back the route it has, which ends beside the destination — at whichever cell it happened to be taking up at that moment, not at the nearest or the best one. Two things fall out of that. A route consisting of nothing but the cell the object is already in is rejected, so an object standing beside a prohibited destination is left with no route at all. And a [`Passive=yes`](/keys/passive/) vehicle takes no such offer, which that key owns.

## Straightening the finished route

The move list is worked over twice before the object is given it, and both passes can change the ground the route covers.

The first looks for a corner where two diagonal runs meet at a right angle and replaces the pair with a straight run, of the same number of steps and ending in the same cell. The straight run is tested cell by cell and is taken only where every cell of it is clear outright — not merely cheap — is unmarked by claimed traffic, and is unthreatened. Where it does not fit, the attempt is shifted one step along and shortened by one, until it fits or is given up on. A corner involving a tunnel step is never touched.

The second looks only at the first twenty steps. It follows how far the route has carried the object from the point the current leg began, and where a step fails to carry it further, that wandering tail is thrown away and replotted as a two-leg run — one diagonal leg and one straight leg, in whichever of the two orders fits. Every cell of the replot must be clear outright and unmarked as before, with a threat allowance that [`ThreatAvoidanceCoefficient`](/keys/threatavoidancecoefficient/) owns, and where neither order is clear the replot is abandoned and the original steps stand. A successful one can shorten the route. Nothing past the twentieth step is examined.

## Settings and state without effect

Three switches inside the search are set once, when the pathfinder is created, and never moved again. Nothing assigns any of them: no rules key reaches them, no saved game carries them, and there is one pathfinder for the whole game.

- **Bridge avoidance** is off. The step cost carries a branch that would multiply a step onto a bridge by ten where the span does not continue and by two where it does, so that routes shy away from bridges; the branch never runs and a bridge step is priced exactly like a ground step.
- **The cost multiplier** is fixed at `1`. Every step's price passes through it before the direction sliver is added, and it changes nothing. It is the figure that would let what stands in the way weigh more heavily against distance.
- **The locomotor question** is on, and this one leaves nothing unreachable. A vehicle's per-step test ends by putting the cell to the object's own travel routine, and the switch decides whether it bothers; the search always has it ask. [Nine of the ten locomotors accept every cell](/systems/movement-and-terrain/#why-a-cell-refuses-a-vehicle) and the tunnelling one does not, so what is actually settled here is that a tunnelling vehicle is held to the burrowing test while its route is being plotted as well as while it drives. Taking the cell without asking is the setting several parts of the game outside movement use, and never the one the search uses.

Two figures the search is handed are not read. The one routine that asks for a route names a ceiling of 2,000 steps on the list it will accept, and names a clear cell as the worst verdict it is prepared to walk into; neither reaches the search, which sizes the route by what it finds and prices worse verdicts rather than refusing them. The search also records the type's [`Speed=`](/keys/speed/) as it starts and consults it nowhere.

:::danger[A route of 2,000 cells or more overruns the list it is written into]
The list handed to the search holds 2,000 entries, and the ceiling named alongside it is the figure that would have bounded the write. A route of 2,000 cells fills that list exactly — 1,999 steps and the marker that ends the list — and the second straightening pass then finishes by filling the tail of the list with that marker and fills one entry past the end. Every further cell on the route puts another entry past the end again, over the storage of the routine that asked for the route. The heights that go with the route are kept in a separate list of the same size, and a route of 2,002 cells or more runs off the end of that one as well.

Nothing holds a route to 2,000 cells. A single pass may take up more than thirty times that many, so a map on which the walk between two points has to wind far enough to run 2,000 cells produces one.

A route that long cannot fit in a corridor of fewer than 500 blocks, and ordinarily needs one of about a thousand. Where a corridor is in force it has therefore given way first, and this case is what is left for a route plotted without one.
:::
