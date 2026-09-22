---
title: Sidebar and build queue
summary: "Lists what the player can build as two strips of cameos, shows each order's progress on its cameo, and turns clicks into production orders."
category: interface-controls
keys:
  - Cameo
  - CameoSortOrder
  - CreditTicks
  - MaximumQueuedObjects
  - RecheckPrerequisites
  - ScoldSound
  - SidebarCameoText
  - SidebarSorting
  - SortCameoAsBaseDefense
related:
  - type: system
    id: production
  - type: system
    id: power
  - type: system
    id: superweapons
  - type: system
    id: map-visibility
  - type: command
    id: ToggleRadar
---

The sidebar is the fixed-width panel along the right edge of the screen. From the top down it holds the credit readout, the radar pane, four mode buttons, and two strips of cameos with the power bar running down their left side. The panel stays up for the whole match. An observer gets the panel with both strips empty.

## What the strips list

A cameo appears on a strip when one of the player's structures offers its type. A structure makes offers only while it is on the map, discovered by the player, switched on, and not being sold or queued to be sold.

Each such structure offers a whole category at once: every type in the list its [`Factory=`](/keys/factory/) names (`[BuildingTypes]`, `[VehicleTypes]`, `[InfantryTypes]` or `[AircraftTypes]`). A type reaches the strip only if [the house may build it](/systems/production/#what-a-house-may-build) and some structure the house owns could produce it. The build limit is the one exception: a type the house already owns as many of as a positive [`BuildLimit=`](/keys/buildlimit/) allows still reaches the strip, darkened. A type already on the strip is not added again.

The left strip holds structures. The right strip holds everything else: vehicles, infantry, aircraft and [superweapon cameos](/systems/superweapons/#the-sidebar-cameo).

### The order of the strips

By default each strip is kept in a fixed order that depends only on the rules, so the same rules give the same strip whatever order the cameos were added in. Entries are compared on each of the following in turn, and the first difference decides. No two entries can tie on all four.

1. **Kind.** Superweapons, then infantry, then aircraft, then vehicles, then structures. Only the right strip holds more than one kind.
2. **[`CameoSortOrder=`](/keys/cameosortorder/)**, lowest first.
3. **Group**, for structures only: ordinary buildings, then walls, then gates, then base defenses. A type that fits more than one group takes the first of them in this order.
   - A wall is a type with [`Wall=`](/keys/wall/#scope-buildingtype), [`FirestormWall=`](/keys/firestormwall/), [`LaserFence=`](/keys/laserfence/) or [`LaserFencePost=`](/keys/laserfencepost/).
   - A gate is a type with [`Gate=`](/keys/gate/).
   - A base defense is a type with [`SortCameoAsBaseDefense=`](/keys/sortcameoasbasedefense/). When that key is absent, it takes the type's [`IsBaseDefense=`](/keys/isbasedefense/#scope-buildingtype) value.
4. **Declaration order**, the type's place in `[BuildingTypes]`, `[VehicleTypes]`, `[InfantryTypes]`, `[AircraftTypes]` or `[SuperWeaponTypes]`.

Both keys that place a structure's cameo are written in its type section:

```ini title="rules.ini"
[SOMEBUILDING]              ; a BuildingType
CameoSortOrder=10           ; where this type's cameo sits among the structures
SortCameoAsBaseDefense=yes  ; group it with the base defenses
```

[`SidebarSorting=no`](/keys/sidebarsorting/) turns this order off. Each new cameo then goes to the end of its strip, and types offered together by one structure keep their declaration order.

A strip is sorted again whenever a cameo is added to it, and after a saved game is loaded, because the rules or the sorting setting may have changed since the save. If the strip has been scrolled, the cameo on its top row stays on the top row. Sorting does not affect production in progress.

Adding a cameo plays the new-construction-options announcement. Superweapon cameos are added without it, and so is every cameo added while a scenario is still being set up.

Each strip holds up to 225 cameos and shows at most 60 at a time. The arrows below the strip scroll through the rest. A type offered to a full strip is left off it, whatever its place in the order. The right strip is the one likely to fill, because vehicles, infantry, aircraft and superweapons all share it.

### What removes a cameo

The strips are checked again after events that can change what the player may build, and a cameo is removed if its type fails the check. This check is looser than the one that added the cameo. Of the four gates in [what a house may build](/systems/production/#what-a-house-may-build), it applies only the build limit, unless [`RecheckPrerequisites=yes`](/keys/recheckprerequisites/) makes it apply all four.

A cameo is removed when **Any of** these is true:

- No structure the player owns could produce the type. A structure counts when it meets **All of**:
  - it is on the map;
  - it is not being sold or queued to be sold;
  - its [`Factory=`](/keys/factory/) names the type's kind;
  - its [`Owner=`](/keys/owner/) overlaps the type's.
- The type's [`BuildLimit=`](/keys/buildlimit/) is zero or below and has been used up.
- With `RecheckPrerequisites=yes`, the house may no longer build the type.

The factory test is [the factory search](/systems/production/#what-counts-as-a-factory) without its switched-on requirement; the construction-yard clause still applies. Switching off every factory of a category therefore darkens those cameos but does not remove them. The test fails once the house has no such structure left, for example after the last one is destroyed or captured.

A superweapon cameo is removed when no structure the house owns supplies that superweapon any more.

:::caution[Losing a prerequisite leaves the cameo in place]
With `RecheckPrerequisites` off, selling or losing a structure named in a type's [`Prerequisite=`](/keys/prerequisite/) does not remove, darken or disable that type's cameo. A click on it still starts production, because the check made when production starts also skips prerequisites. A house can keep building a type whose prerequisite is gone.

With [`RecheckPrerequisites=yes`](/keys/recheckprerequisites/), the cameo is removed at the next check. Every copy of that type the factory holds, queued or in production, is canceled with it.
:::

When a cameo is removed, the rest of the strip closes up. The topmost cameo that was visible before the removal and is still on the strip stays on the row it occupied, as far as the strip's length allows. If none of the visible cameos is left, the strip returns to its top.

## What a cameo shows

A cameo is drawn from the art [`Cameo=`](/keys/cameo/) selects. It is darkened when **Any of** these applies:

- it is a structure cameo and the house has any structure order outstanding, which [the queue](/systems/production/#the-queue) covers;
- no switched-on structure could produce the type;
- the house owns as many of the type as a positive `BuildLimit=` allows, which [build limits](/systems/production/#build-limits) covers.

The cameo of the type in production is never darkened, whether its order is building, on hold or finished.

A darkened cameo still answers clicks. What happens depends on why it is dark:

- A structure cameo darkened by an outstanding structure order refuses a left click; see [Clicking a build cameo](#clicking-a-build-cameo).
- For the other two causes, a left click behaves as [the table below](#clicking-a-build-cameo) describes, but the order it sends is dropped because no structure can take it. A right click still removes a queued copy.

Those dropped orders do not play [`ScoldSound`](/keys/scoldsound/). That sound plays only when [the queue](/systems/production/#the-queue) refuses an order.

An empty cameo slot draws nothing, so the backdrop shows through it.

A superweapon cameo follows the superweapon's charge instead of production. It is never darkened, never shows a count, and a click on it never places a production order. [The superweapon cameo](/systems/superweapons/#the-sidebar-cameo) covers what it shows and what clicking it does.

### While an order runs

While a type is in production, its cameo shows a build clock from `GCLOCK2.SHP`. The clock frame is the order's [production step](/systems/production/#production-steps) plus one, so the 54 steps use frames 1 through 54 and frame 0 is never drawn. When the order finishes, the ready caption replaces the clock. An order on hold keeps its clock and adds the hold caption.

The top-right corner of a cameo shows how many of that type are outstanding: the one in production plus every queued copy. The count appears when it is 2 or more, and when it is 1 and that one is still waiting in the queue. It can reach at most one more than [`MaximumQueuedObjects`](/keys/maximumqueuedobjects/). When a count and the hold caption share the top row, the hold caption moves to the left edge.

### Captions and tooltips

With [`SidebarCameoText=yes`](/keys/sidebarcameotext/), each cameo is captioned with its name. A name wider than the cameo is wrapped at spaces and hyphens, with earlier lines stacked above later ones. The caption never shows the price.

The tooltip carries the price. It shows the price alone while captions are on, and the name and price while they are off. A superweapon's tooltip shows only its name.

## Clicking a build cameo

Both mouse buttons act when pressed, not when released.

Structures, vehicles, infantry and aircraft each have their own production line. Each row below describes what the clicked cameo's line has on order at the time of the click.

| What the cameo's production line holds | Left click | Right click |
| --- | --- | --- |
| Nothing on order | Starts it, announced | Nothing |
| Another type on order (building, on hold, finished or queued) | Queues it, silently | Removes one queued copy of this type, if it has one |
| This type building | Queues another, silently | Puts the build on hold, announced |
| This type on hold | Resumes it, announced | Removes a queued copy first if one exists, otherwise cancels and refunds it; announced either way |
| This type finished (vehicle, infantry or aircraft) | Asks the factory to let it out | Removes a queued copy first if one exists, otherwise cancels and refunds it; announced either way |
| This type finished (structure) | Enters placement mode | Cancels and refunds it, announced, and leaves placement mode |
| This type finished, no factory left to build it | Removes a queued copy first if one exists, otherwise cancels and refunds it; announces that there is no factory either way | Removes a queued copy first if one exists, otherwise cancels and refunds it; announced either way |

Queuing plays no announcement, so a second click on a cameo that is already building seems to do nothing until the count appears.

A right click never starts anything. Where it cancels, it removes a queued copy before touching the object in production, so one right click undoes one extra left click.

A right click on any cameo whose type is in production also leaves placement mode, even when the structure being placed belongs to another cameo.

Structures are never queued, so the queuing rows do not apply to them. While the house has any structure order outstanding (building, on hold, or finished and waiting to be placed), a left click on any *other* structure cameo is refused. The engine announces that there is no factory and sends no order.

The cameo of the outstanding structure order still answers: a left click resumes the order when it is on hold and enters placement mode when it is finished. While that structure is building, a left click on its own cameo is refused like the others.

## Scrolling the strips

The two arrows below each strip scroll that strip alone. A left click moves it one row and a right click moves it a screenful. Clicking an arrow that has nowhere left to go plays [`ScoldSound`](/keys/scoldsound/).

Twelve commands do the same from the keyboard. Each motion has three commands: one moves both strips, and the other two move one strip each.

| Motion | Both strips | Structures only | Everything else only |
| --- | --- | --- | --- |
| One row up | [Sidebar Up](/commands/sidebarup/) | [Structure List up](/commands/leftsidebarup/) | [Unit List Up](/commands/rightsidebarup/) |
| One row down | [Sidebar Down](/commands/sidebardown/) | [Structure List Down](/commands/leftsidebardown/) | [Unit List Down](/commands/rightsidebardown/) |
| A screenful up | [Sidebar PageUp](/commands/sidebarpageup/) | [Structure List PageUp](/commands/leftsidebarpageup/) | [Unit List PageUp](/commands/rightsidebarpageup/) |
| A screenful down | [Sidebar PageDown](/commands/sidebarpagedown/) | [Structure List PageDown](/commands/leftsidebarpagedown/) | [Unit List PageDown](/commands/rightsidebarpagedown/) |

Turning the mouse wheel forward runs Sidebar Up, and turning it back runs Sidebar Down, wherever the pointer is.

:::caution[When the scroll commands play ScoldSound]
The four both-strips commands, including the mouse wheel, play `ScoldSound` only when neither strip can move. A strip already at its end stays silent as long as the other one still moves. The eight one-strip commands never play it, although the arrow buttons for the same motion do.
:::

A strip moves one whole row per update, so a screenful takes one update per row. A partly scrolled row is never drawn.

## The rest of the panel

### The power bar

The bar's height follows the total rated output and drain of the player's structures. The larger that total, the closer the bar comes to the full height of the strips, and it always shows at least one pip. [The power page](/systems/power/#player-feedback) covers why those ratings and the house's actual power tally can disagree.

The colors come from the actual tally. Red is the power being consumed, yellow is the first 100 units of surplus, and green is any surplus beyond that. A house whose output is below its drain has no surplus, so its bar is entirely red. Rounding leftovers from the three bands are also added to red.

When the figures change, the bar adjusts one pip at a time: the red band first, then green, then yellow. A growing bar slows down as it nears its target height. Any change to output or drain also blinks a white pip five times at the top of the colored pips, starting once the bar has stopped moving.

Hovering over the bar shows the house's output and drain.

### The credit readout

The readout counts toward the house's money instead of jumping to it. Each step closes one eighth of the remaining gap, moving at least 1 and at most 143 credits. The readout takes a step every update while rising, but only every third update while falling, so money leaves the display three times more slowly than it arrives. It never shows less than zero.

Each step plays a [`CreditTicks`](/keys/creditticks/) sound at half volume: the first entry while the figure rises, and the second while it falls.

While a scenario's mission timer runs, the timer is shown beside the readout. A reminder is spoken at exactly 20, 10, 5, 4, 3, 2 and 1 minutes remaining.

For an observer, the readout shows the elapsed match time instead and plays no ticks. [Observers and coach mode](/systems/observers/) covers that display.

### The radar pane

The pane sits below the credit readout. [The radar map](/systems/map-visibility/#the-radar-map) covers what raises and lowers it and what it draws while showing the map. Besides the map, the pane can show the multiplayer name and kill list, an in-game movie, or its blank frame.

With something selected, a left click in the radar picture can act as a click on that cell in the tactical view. It does so when the selection's action there is a move, blocked move, attack, enter, capture, sabotage or harvest. Any other left click, including every click with nothing selected, recenters the tactical view on that cell. The new view is kept within the playable area.

:::caution[Radar Toggle does nothing in a campaign]
[Radar Toggle](/commands/toggleradar/) switches the pane between the multiplayer name and kill list and the radar map, or the blank frame when the player has no radar. In a campaign game the command does nothing. Nothing else shows the name and kill list, so a campaign never shows it.
:::

### The mode buttons

The four buttons above the strips toggle the same modes as [Repair Mode](/commands/togglerepair/), [Sell Mode](/commands/togglesell/), [Power Mode](/commands/togglepower/) and [Waypoint Mode](/commands/waypointmode/), in that order from the left.

## What is fixed in the engine

No setting changes the panel's layout. Its width, the number of strips, their positions, the size of a cameo slot and the 225-entry capacity are fixed in the engine. So are the one-row scroll step, the timing of the power bar's blink, and the timing of the radar animation.

The panel always sits on the right edge of the screen. Neither `sun.ini` nor the rules can move it.

The number of cameo slots a strip shows depends on the screen height. It is the panel height left after the backdrop's top piece and bottom cap, divided by the height of the repeating middle piece, up to a maximum of 60. A taller screen therefore shows more cameos and a shorter one fewer. On a screen with room for more than 60 rows, the backdrop ends below the sixtieth slot and does not reach the bottom of the screen.

The art filenames are fixed as well. The table lists each file the panel loads and what it draws.

| File | What it draws |
| --- | --- |
| `SIDE1.SHP`, `SIDE2.SHP`, `SIDE3.SHP` | The backdrop's top piece, its repeating middle piece and its bottom cap |
| `ADDON.SHP` | The trim panel below the bottom cap |
| `R-UP.SHP`, `R-DN.SHP` | The scroll arrows, shared by both strips |
| `REPAIR.SHP`, `SELL.SHP`, `POWER.SHP`, `WAYP.SHP` | The four mode buttons |
| `GCLOCK2.SHP`, `RCLOCK2.SHP` | The build clock and the recharge clock |
| `DARKEN.SHP` | The overlay drawn over a darkened cameo |
| `XXICON.SHP` | The cameo used when `Cameo=` is absent or names a file that cannot be found |
| `POWERP.SHP` | The power bar's pips |
| `RADAR.SHP` | The radar frame and its open and close animation |
| `TABS.SHP` | The tab bar and the credit readout's backdrop |
| `SIDEBAR.PAL`, `CAMEO.PAL` | The palettes for the panel's art and for the cameos |

Each side has its own numbered set of archives, and they give the panel its per-side look; no key is involved. When the player's side is set up, the engine unmounts the previous side's archives, mounts the new side's, and loads the backdrop, mode buttons, scroll arrows, `SIDEBAR.PAL`, power pips, radar frame and tab art again. Every cameo, including the `XXICON.SHP` fallback, is also fetched again after the new side's archives are mounted. A copy of any of these files in a side's archives therefore changes the panel for that side.

The clock, darken and `CAMEO.PAL` art is loaded once at startup and does not change with the side.

## Parsed settings without effect

[`TimerWarning`](/keys/timerwarning/) is read but changes nothing. The mission timer's tab looks the same whether or not the time left is inside that many minutes.
