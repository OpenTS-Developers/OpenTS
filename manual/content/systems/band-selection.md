---
title: Band selection
summary: "Dragging the left button across the tactical view selects the player's objects inside the box."
category: interface-controls
keys:
  - Selectable
  - UndeploysInto
  - IsMobileWar
---

## Starting a band

A left-button press on the tactical view can start a selection box. No box starts while the player is in repair, power or sell mode, is targeting a superweapon, or has a structure waiting to be placed.

The box appears once the pointer has moved further from the press point than the Windows drag distance, on either axis. That distance follows the display scale and the pointer accessibility settings, so it is not a fixed number of pixels. Coast scrolling on the right button starts at twice the same distance.

While the button is held, the far corner of the box follows the pointer and stops at the edge of the tactical view. The map does not scroll at the screen edge while the left button is held.

In waypoint mode no box appears and a drag selects nothing. Releasing the button still clears the current selection unless left Shift is held.

Releasing the right button while the left is still held abandons the box, so the later left release selects nothing. That right release also clears the current selection, as any right click with no mode active does.

The box is also abandoned, and selects nothing, when the in-game menu opens, when a trigger action or the end of a campaign mission locks the player's input, or when another window takes the mouse.

## What the box takes

Releasing the button clears the current selection, unless left Shift is held.

An object's drawing position is the single point on screen where it is drawn, not the cells it covers. An object whose art overlaps the box is not selected unless that point lies inside it.

The box then selects each object whose drawing position lies inside it and that meets **all of**:

- the player owns it;
- its type has [`Selectable=yes`](/keys/selectable/);
- it is not a structure, or it is a structure that [undeploys into a vehicle](/keys/undeploysinto/) and is neither a [construction yard](/keys/constructionyard/) nor an [`IsMobileWar=yes`](/keys/ismobilewar/) structure;
- it is out of [limbo](/glossary/#limbo);
- it is not a loaner, meaning an object the scenario only lends the player. A loaner the player cannot currently move, such as one stunned by an [EMP pulse](/systems/emp-pulse/), is taken.

A deployed artillery piece or tick tank is therefore selected by a box drawn over it. A construction yard, a deployed mobile war factory and every structure that does not undeploy are not.

Only the first object the box selects plays its selection response. The [Selected by player](/mapping/events/tevent-selected/) trigger event also springs for that first object only.
