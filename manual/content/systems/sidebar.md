---
title: Sidebar and build queue
summary: "Draws a house's build options as two cameo strips, marks each order's progress on its own cameo, and turns a click into a production order."
category: interface-controls
keys:
  - Cameo
  - CameoSortOrder
  - CreditTicks
  - MaximumQueuedObjects
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

The panel fills a fixed-width column against the right edge of the screen. From the top down it holds the credit readout, the radar pane, four mode buttons, and the power bar running down the left of two strips of cameos. It is switched on again on every update, so it stays up for the whole match. An observer keeps the panel with nothing on either strip.

## What the strips list

Every structure the player owns that is on the map, discovered and switched on offers a whole category at once. The engine reads that structure's [`Factory=`](/keys/factory/) and walks the type list for the kind it names (`[BuildingTypes]`, `[VehicleTypes]`, `[InfantryTypes]` or `[AircraftTypes]`), adding every type on it that [the house may build](/systems/production/#what-a-house-may-build) and that some structure the house owns could produce. A type already listed is not added twice.

The left strip holds structures. The right strip holds everything else: vehicles, infantry, aircraft and [superweapon cameos](/systems/superweapons/#the-sidebar-cameo).

### The order of the strips

Each strip is put in order before it is next drawn, so the same rules set gives the same strip whichever order the offers arrived in. Entries are compared on each of the following in turn, and the first difference settles the order. No two entries can be tied on all four.

1. **Kind.** Superweapons, then infantry, then aircraft, then vehicles, then structures. Only the right strip holds more than one kind.
2. **[`CameoSortOrder=`](/keys/cameosortorder/)**, the type's own number, lowest first.
3. **Group**, structures only: ordinary buildings, then walls, then gates, then base defenses. A wall is a type with [`Wall=`](/keys/wall/#scope-buildingtype), `FirestormWall=`, `LaserFence=` or `LaserFencePost=`. A gate is one with [`Gate=`](/keys/gate/), and a base defense is one with [`SortCameoAsBaseDefense=`](/keys/sortcameoasbasedefense/). That key takes its value from [`IsBaseDefense=`](/keys/isbasedefense/#scope-buildingtype) unless it is given one of its own. A type matching more than one group is taken as the first of them in the order above.
4. **Declaration order**, the type's place in `[BuildingTypes]`, `[VehicleTypes]`, `[InfantryTypes]`, `[AircraftTypes]` or `[SuperWeaponTypes]`.

[`SidebarSorting=no`](/keys/sidebarsorting/) leaves each strip in the order the offers arrived instead. Within one offer that is the order the types are declared in; across offers it is the order the offering structures matured. The two keys below place a type's cameo in a sorted strip and are written in the type's section:

```ini title="rules.ini"
[SOMEBUILDING]              ; a BuildingType
CameoSortOrder=10           ; where this type's cameo sits among its kind
SortCameoAsBaseDefense=yes  ; group it with the base defenses
```

The capacity below still admits types in the order they arrive, so no ordering decides which of them a full strip holds.

A strip is put back in order whenever a cameo is added to it, and again when a saved game is loaded. The rules and the setting may both have moved since the game was saved. A strip that has been scrolled keeps whichever cameo was on its top row there. Reordering moves no production.

A type reaching a strip for the first time speaks the new-construction-options line. A superweapon cameo is always added silently, and so is any cameo added while a scenario is still being set up.

A strip holds 225 entries and shows at most 60 of them at a time, so the arrows below it scroll through the rest. A type offered to a strip that is already full is silently left off it. The right-hand strip is the one to watch, because it holds every vehicle, infantry, aircraft and superweapon a house may build at the same time.

### What removes a cameo

The pass that revalidates the strips asks a much narrower question than the one that put a cameo there. It skips tech level, prerequisites and ownership outright and drops straight to the build limit. A cameo therefore leaves a strip on **Any of** these tests:

- the player owns no structure that could produce that kind of object — **All of:** it is on the map, it is neither being sold nor queued to be sold, its [`Factory=`](/keys/factory/) names that kind of object, and its [`Owner=`](/keys/owner/) overlaps the object's;
- the type's [`BuildLimit=`](/keys/buildlimit/) is zero or below and has been spent.

That first test is [the factory search](/systems/production/#what-counts-as-a-factory) without its switched-on check, construction-yard clause included. That is why switching every factory of a category off darkens those cameos without removing them. A superweapon's cameo leaves when no structure the house owns supplies it any more.

The same factory search runs before a cameo is added, so a type no structure the house owns could produce never reaches a strip at all. It leaves one only when the structure that could have produced it is lost.

:::caution[Losing a prerequisite leaves the cameo in place]
Selling or losing the structure a type names in [`Prerequisite=`](/keys/prerequisite/) does not remove that type's cameo, does not darken it, and does not stop it answering a click. The order that click sends is accepted as well, because the check made when production starts skips prerequisites in the same way. A house can go on building from a cameo whose prerequisite is rubble.
:::

A removal closes the gap in the strip. The strip then tries to keep whichever of the previously visible entries survived on the row it already occupied. When none survived, it returns to its top.

## What a cameo shows

A buildable cameo is drawn at full brightness from the art [`Cameo=`](/keys/cameo/) selects. Three independent conditions each lay the darkening shape over it instead:

- any structure order outstanding darkens every structure cameo at once, which [the queue](/systems/production/#the-queue) covers;
- no switched-on structure could produce that kind of object, so switching off every factory of a category darkens that category's whole set of cameos while leaving them in place;
- the type's owned count has reached a positive `BuildLimit=`, which [build limits](/systems/production/#build-limits) covers.

An order in progress overrides all three. Starting one links the house's [production slot](/systems/production/) to the cameo slot of the exact type it is building, and a linked cameo slot is drawn at full brightness whatever the three conditions say.

Darkening is cosmetic: the cameo still answers a click. A structure cameo darkened because a structure order is outstanding is refused at the panel itself. The engine announces that there is no factory and sends no order at all. The other two causes do send the order, which is then dropped in silence when no structure can take it. Neither of those two causes plays [`ScoldSound`](/keys/scoldsound/). That sound answers the queue gate (a full queue, or a type already at its build limit), which [the queue](/systems/production/#the-queue) covers.

An empty cameo slot draws no art at all, so the backdrop shows through it.

A superweapon's cameo slot is driven by its own charge state rather than by production. It is never darkened, never shows a count, and never answers a click with a production order. [The superweapon cameo](/systems/superweapons/#the-sidebar-cameo) covers what it shows and what clicking it does.

### While an order runs

A cameo slot with the production slot linked to it shows a clock drawn from `GCLOCK2.SHP` over the cameo. The clock is drawn at the frame one past the order's [production step](/systems/production/#production-steps). The 54 steps therefore use frames 1 through 54, and frame 0 is never drawn. The last step replaces the clock with the ready caption. An order put on hold keeps its clock and adds the hold caption.

A count is printed in the cameo's top right corner when more than one of that type is outstanding. It is printed too when exactly one is outstanding and it is not the object currently being built. It counts the object under construction plus every queued copy of the same type, so it never exceeds one more than [`MaximumQueuedObjects`](/keys/maximumqueuedobjects/). When a count and a hold caption share the row, the caption moves to the left edge.

### Captions and tooltips

With [`SidebarCameoText=yes`](/keys/sidebarcameotext/) each cameo is captioned with the object's name. The name is wrapped to the slot width, broken at spaces and hyphens, with earlier lines stacked upward. The caption shows the name alone. The price lives in the tooltip, which reports the price by itself while captions are on and the name and price together while they are off. A superweapon's tooltip is its name in either case.

## Clicking a build cameo

Both mouse buttons act on press rather than on release. The table gives what each button does for every state the cameo's kind of object can be in. The rule behind the right-hand column is that a right click never starts anything. Wherever it cancels, it takes a queued copy first.

| State of the cameo's kind of object | Left click | Right click |
| --- | --- | --- |
| Nothing of its kind on order | Starts it, announced | Nothing |
| Another type of its kind building or queued | Queues it, silently | Removes one queued copy of this type, if it has one |
| This type building | Queues another, silently | Puts the build on hold, announced |
| This type on hold | Resumes it, announced | Removes a queued copy first if one exists, otherwise cancels and refunds it; announced either way |
| This type finished (vehicle, infantry or aircraft) | Asks the factory to let it out | Removes a queued copy first if one exists, otherwise cancels and refunds it; announced either way |
| This type finished (structure) | Enters placement mode | Cancels and refunds it, and clears the placement cursor |
| This type finished, no factory left to build it | Cancels the order and announces that there is no factory | Removes a queued copy first if one exists, otherwise cancels and refunds it; announced either way |

Queuing is silent, so a second click on a cameo already building reads as nothing having happened until the count appears. Taking a queued copy before the object under construction is what makes the right click the way to undo a single click of over-ordering.

Structures skip the queuing rows entirely, because they are never queued. While a house has any structure order outstanding (building, on hold, or finished and waiting to be placed), a left click on any *other* structure cameo is refused on the spot. The engine announces that there is no factory and sends nothing at all. The cameo the order belongs to still answers normally: it resumes an order on hold and enters placement mode for a finished one. Only while it is actively building is it refused like the rest.

## Scrolling the strips

The two arrows below each strip move that strip alone: a left click moves it one row, a right click a whole screenful. An arrow with nowhere left to go plays [`ScoldSound`](/keys/scoldsound/). Twelve commands cover the same two motions from the keyboard. The table sets each motion against three commands: one that applies it to both strips, and two that apply it to one strip alone. The column a command sits in is what decides which strip moves.

| Motion | Both strips | Structures only | Everything else only |
| --- | --- | --- | --- |
| One row up | [Sidebar Up](/commands/sidebarup/) | [Structure List up](/commands/leftsidebarup/) | [Unit List Up](/commands/rightsidebarup/) |
| One row down | [Sidebar Down](/commands/sidebardown/) | [Structure List Down](/commands/leftsidebardown/) | [Unit List Down](/commands/rightsidebardown/) |
| A screenful up | [Sidebar PageUp](/commands/sidebarpageup/) | [Structure List PageUp](/commands/leftsidebarpageup/) | [Unit List PageUp](/commands/rightsidebarpageup/) |
| A screenful down | [Sidebar PageDown](/commands/sidebarpagedown/) | [Structure List PageDown](/commands/leftsidebarpagedown/) | [Unit List PageDown](/commands/rightsidebarpagedown/) |

:::caution[The refusal sound follows the request, not the strip]
The four both-strips commands play `ScoldSound` only when neither strip could move. A strip already at its end therefore stays silent as long as the other one still moves. The six one-strip commands never play it at all: the same refusal that scolds from an arrow button is silent from the keyboard.
:::

An arrow click is carried out in the update that delivers it. A keyboard request is carried out on the next one. Either way a strip travels exactly one cameo slot's height per update, so a page walks a row per update. The strips jump a whole row at a time and no partial offset is ever drawn. The gradual slide the drawing path still allows for is never seen.

## The rest of the panel

### The power bar

The bar's height grows toward the full height of the strip area as the player's structures add rated output and drain. [The power page](/systems/power/#player-feedback) covers why those ratings and the tally that runs the game can disagree. The bands come from the real tally: red is the power being consumed, yellow the first 100 units of surplus above it, and green everything beyond that. Every truncation remainder is added back into red, so a house whose output is below its drain gets a bar that is entirely red. The bar moves one pip per step, red first, then green, then yellow, at a pace worked out from its own height. Any change to output or drain also blinks a white pip five times at the top of the colored run, and the blinking holds off until the pips have finished moving. Hovering over the bar reports the house's output and drain figures.

### The credit readout

The readout shows a running figure that walks toward the house's actual money rather than jumping to it. Each step closes an eighth of the remaining gap, with a floor of 1 and a ceiling of 143. A step is taken on every update while the figure rises, but only every third while it falls, so money drains from the display three times more slowly than it fills. Every step plays a [`CreditTicks`](/keys/creditticks/) sound at half volume. The first entry plays while the figure rises and the second while it falls. The figure never shows less than zero. A scenario's mission timer prints beside it while it runs and speaks a reminder at exactly 20, 10, 5, 4, 3, 2 and 1 minutes remaining. For an observer the readout shows the elapsed match time instead and plays no tick; [observers and coach mode](/systems/observers/) owns that display.

### The radar pane

The pane sits below the credit readout. What raises and lowers it, and what it draws while it is showing the map, are covered by [the radar map](/systems/map-visibility/#the-radar-map). Beyond that map the pane has three other displays: the multiplayer name and kill list, an in-game movie, and its blank frame.

A click inside the radar picture with something selected issues an order when the cell under the cursor resolves to a move, a blocked move, an attack, an enter, a capture, a sabotage or a harvest. Every other action is discarded rather than passed on. With nothing to order, a left click recenters the tactical view on that cell, clamped so the view stays over the playable area.

:::caution[The radar mode cycle does nothing in a campaign]
[Radar Toggle](/commands/toggleradar/) changes what the pane shows, and its whole body is skipped in a campaign game, so the command has no effect there at all. The multiplayer name and kill list is reachable only through that command, which puts it out of reach in a campaign as well.
:::

### The mode buttons

The four buttons above the strips toggle the same modes as [Repair Mode](/commands/togglerepair/), [Sell Mode](/commands/togglesell/), [Power Mode](/commands/togglepower/) and [Waypoint Mode](/commands/waypointmode/), in that order from the left.

## What is fixed in the engine

Almost nothing on this surface is laid out from a setting. The panel's side, its width, the number of strips, their positions, the size of a cameo slot and the 225-entry capacity are all compile-time constants. So are the distance a strip scrolls in one update, the pacing of the power bar's blink, and the pacing of the radar animation.

The panel occupies the right edge of the screen and nothing moves it. The engine still holds a complete left-hand alternative for the sidebar, the tab bar, the radar and the tooltip regions. Which of the two is used is fixed in code and never read from `sun.ini` or from rules, so the left-hand layout is unreachable.

The one figure that does vary is how many cameo slots a strip shows, and it is not a setting either. It is the height left over after the backdrop's top piece and bottom cap, divided by the height of its repeating middle piece. A taller screen therefore shows more cameos and a shorter one fewer.

A strip stops at 60 slots however tall the screen is. The backdrop is built from that same count. On a screen with room for more than 60 rows, the sidebar art ends below the sixtieth slot rather than at the foot of the screen.

None of the art filenames comes from a setting either. The table gives each file the panel loads and what that file draws. Which of them resolve to different art for each side is settled by the paragraph below it rather than by anything in the rules.

| File | What it draws |
| --- | --- |
| `SIDE1.SHP`, `SIDE2.SHP`, `SIDE3.SHP` | The backdrop's top piece, its repeating middle piece and its bottom cap |
| `ADDON.SHP` | The trim panel below the bottom cap |
| `R-UP.SHP`, `R-DN.SHP` | The scroll arrows, shared by both strips |
| `REPAIR.SHP`, `SELL.SHP`, `POWER.SHP`, `WAYP.SHP` | The four mode buttons |
| `GCLOCK2.SHP`, `RCLOCK2.SHP` | The build clock and the recharge clock |
| `DARKEN.SHP` | The overlay drawn over an unavailable cameo |
| `XXICON.SHP` | The cameo used when the configured one cannot be loaded |
| `POWERP.SHP` | The power bar's pips |
| `RADAR.SHP` | The radar frame and its open and close animation |
| `TABS.SHP` | The tab bar and the credit readout's backdrop |
| `SIDEBAR.PAL`, `CAMEO.PAL` | The palettes the panel's own art and the cameos are drawn through |

The per-house look comes from mounting archives rather than from any key. Preparing a side releases the archives mounted for the previous side and mounts the numbered set belonging to the new one. It then re-fetches the backdrop, buttons, arrows, palette, power pips, radar frame and tab art, so those names resolve to different art for each side. The clock, darken, fallback-cameo and cameo-palette art is fetched once at startup and does not change with the side.

## Parsed settings without effect

The mission timer works out from [`TimerWarning`](/keys/timerwarning/) whether the time left has fallen inside its warning window. It then draws the ordinary tab frame either way. The highlighted frame that decision would select is never drawn, and nothing else reads the value.
