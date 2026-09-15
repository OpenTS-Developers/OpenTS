---
title: Waypoint paths
summary: "Lets a player plot up to twelve numbered paths on the map, loop them, edit them, and send objects along them."
category: interface-controls
keys:
  - MaxWaypointPathLength
  - WaypointAnimationSpeed
related:
  - type: command
    id: WaypointMode
  - type: command
    id: DeleteWaypoint
  - type: system
    id: action-lines
  - type: system
    id: sidebar
---

A path is a numbered list of waypoints, each marking one cell. A player lays the path down in waypoint mode and later orders objects along it. Each house holds twelve paths, and a path holds up to [`MaxWaypointPathLength`](/keys/maxwaypointpathlength/) waypoints. Paths belong to the player who plotted them: they are never sent to other machines, and a saved game keeps them.

## Plotting a path

[Waypoint Mode](/commands/waypointmode/), the sidebar's waypoint button, or a click on an existing waypoint starts the mode. Starting it from the command or the button selects the first of the house's paths that holds no waypoints, and is refused when all twelve are in use. A click on an existing waypoint selects that waypoint's path instead. Entering the mode leaves repair, sell and power mode, unselects everything, and is refused while a structure is waiting to be placed.

Inside the mode, a click on an empty cell inside the playable area appends a waypoint to the selected path, snapped to the cell's center and, under a bridge, raised to the deck. A cell already holding a waypoint of another path refuses one; the selected path may cross its own cells. Placing the waypoint that reaches the length limit leaves the mode, and a right click leaves it at any time.

Every path is drawn on the map as its numbered markers joined by lines, the selected path's lines dashed and moving, and [`WaypointAnimationSpeed`](/keys/waypointanimationspeed/) paces the markers' animation.

## Looping a path

A plain click on any waypoint of the selected path but its last closes the loop. After its last waypoint the path returns to the one clicked and goes round again. A looped path takes no further waypoints, and the loop can only be closed while the path can still take them. A path at its length limit therefore cannot be looped.

## Picking up and moving a waypoint

A click with Shift held picks up the waypoint under the cursor. So does a plain click on the last waypoint of the selected path, or on a waypoint of another path, which also makes that path the selected one. The picked-up waypoint follows the cursor over free cells inside the playable area until a click puts it down. A click on a cell that holds another waypoint, or outside the playable area, puts it down where it last stood. Leaving the mode returns it to where it was picked up. An object already heading for the waypoint finishes that leg at the old spot.

## Deleting a waypoint

[Delete Waypoint](/commands/deletewaypoint/) removes the picked-up waypoint or, with none picked up, the last waypoint of the selected path, so pressed repeatedly it takes the path apart from the end. The path stays selected after the mode is left, so the command also works outside it.

The loop survives a deletion: the waypoint the path returns to keeps its place when an earlier waypoint is removed. When the return waypoint itself is removed, the loop returns to the one after it. Removing the last waypoint opens the loop. An object traveling the path keeps heading for the same waypoint, or for the one after a removed waypoint it was heading to.

## Sending objects along a path

An order given by clicking a waypoint's cell with objects selected becomes an order along that path, starting at that waypoint. A move order follows the path, an attack or harvest order attacks along it, an enter or capture order enters along it, and the guard-area order, with Control and Alt held, patrols it. The object then repeats the same click at each waypoint in turn, shifting it to a nearby cell when the waypoint's own cell is occupied or cannot be reached.
