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

A waypoint path is a numbered list of waypoints, each marking one cell. The player lays a path down in waypoint mode, then orders infantry, vehicles, or aircraft along it. Each player has twelve paths, and a path holds up to [`MaxWaypointPathLength`](/keys/maxwaypointpathlength/) waypoints.

Paths exist only on the machine of the player who plotted them, and other players never see them. A saved game keeps them.

## Plotting a path

Waypoint mode starts in one of two ways:

- [Waypoint Mode](/commands/waypointmode/) or the sidebar's waypoint button selects the first path that holds no waypoints. Either is refused when all twelve paths hold waypoints.
- With nothing selected, a click on a waypoint selects that waypoint's path, even when all twelve paths are in use. If a selectable object is under the cursor, the click selects the object instead.

Neither works while a structure is waiting to be placed.

Entering the mode turns off repair, sell, and power mode and unselects everything.

In the mode, a click on a cell in the playable area that holds no waypoint adds a waypoint to the end of the selected path. The waypoint sits at the center of the cell, or on the bridge deck when a bridge crosses the cell. A click on a cell that already holds a waypoint, from any path, adds nothing. It loops the path or picks the waypoint up instead, as described below.

The mode ends when any of these happens:

- The player presses Waypoint Mode or the waypoint button again.
- The player right-clicks.
- A new waypoint brings the path to its length limit.
- The player presses a hotkey that selects, adds to, or centers on a team, or presses [Next Object](/commands/nextobject/) or [Previous Object](/commands/previousobject/).
- A mission trigger locks the player's input.

Every path is drawn on the map in its own color, as markers numbered from 0 and joined by lines. A looped path also has a line from its last waypoint back to the waypoint it returns to. The selected path's lines are dashed and animated. [`WaypointAnimationSpeed`](/keys/waypointanimationspeed/) sets how fast the markers animate.

## Looping a path

A plain click on a waypoint of the selected path loops the path at that waypoint, unless it is the path's last waypoint. After its last waypoint, a looped path returns to the clicked waypoint and repeats from there.

A looped path takes no more waypoints. A path can be looped only while it can still take waypoints, so a path at its length limit cannot be looped.

## Picking up and moving a waypoint

In waypoint mode, a click on a waypoint picks it up when any of these is true:

- Shift is held.
- The waypoint belongs to another path. That path becomes the selected path.
- The waypoint is the last one on the selected path.
- The selected path is looped or at its length limit.

A picked-up waypoint follows the cursor over cells in the playable area that hold no other waypoint. The next click puts it down. A click on a cell that holds another waypoint, or on a cell outside the playable area, leaves the waypoint on the last cell it followed the cursor to. Leaving the mode while holding a waypoint returns it to where it was picked up.

An object heading for a picked-up waypoint keeps going to the waypoint's old position while it is held. Once the waypoint is put down, the object turns toward the new position when it enters its next cell.

## Deleting a waypoint

[Delete Waypoint](/commands/deletewaypoint/) removes the picked-up waypoint. With no waypoint picked up, it removes the last waypoint of the selected path, so pressing it repeatedly takes the path apart from the end.

In waypoint mode, the selected path is the one being edited. Outside the mode, the command shortens the path of the waypoint under the cursor, but only when the cursor there shows a path order or offers to select the path. Otherwise it does nothing. With the cursor off the map, it shortens the path the cursor last pointed at.

Deleting keeps a loop where it can:

- Removing a waypoint before the return waypoint leaves the loop on the same return waypoint.
- Removing the return waypoint moves the loop to the waypoint after it.
- Removing the path's last waypoint opens the loop.

An object traveling the path keeps heading for the same waypoint. If that waypoint is removed, the object turns toward the waypoint after it when it enters its next cell. If the removed waypoint was the path's last, the object finishes its current move and stops following the path.

## Sending objects along a path

With infantry, vehicles, or aircraft selected, a click on a waypoint sends them along that waypoint's path, starting at the clicked waypoint. The cursor over a waypoint shows the order in its path form:

| Order the click would give | Path order |
| --- | --- |
| Move | Follow the path |
| Attack or harvest | Attack along the path |
| Enter or capture | Enter along the path |
| Guard area, with Control and Alt held | Patrol the path |

At each waypoint in turn, an object carries out the order that a plain click on that waypoint's cell would give it at that moment. When it finishes, it goes on to the next waypoint. On a looped path it keeps going round; otherwise it stops after the last waypoint.

Modifier keys held for the original click count only at the clicked waypoint, so a forced attack does not repeat at later waypoints. A patrol order is the exception and holds for the whole path.

An object leaves its path in two ways:

- It cannot carry out the order at a waypoint.
- The player gives it an order on a cell without a waypoint.

An object patrolling the path breaks off to fight threats it finds between waypoints, then resumes its travel.

When another object stands on a waypoint, the follower reacts to it:

- If the occupant is not an ally and a click on it would give an order other than a move, such as an attack, the follower carries out that order.
- If the occupant is an allied structure that repairs or reloads, and the follower needs that service, the follower goes to it. Infantry are never sent for repair this way.
- Otherwise, the follower heads for a free cell near the waypoint if it can reach one, and for the waypoint itself if not.

A group sent along a path with a move or patrol order keeps its spread. Each member is sent to a separate cell around the clicked waypoint. At each later waypoint, it aims for the cell in the same position relative to that waypoint. If it cannot enter or reach that cell, or a click there would give a different order, it aims for the waypoint itself.

A follower that stepped aside from an occupied waypoint keeps that step at later waypoints in the same way.
