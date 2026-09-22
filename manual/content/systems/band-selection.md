---
title: Band selection
summary: "Dragging the left button across the tactical view selects the player's own objects inside the box."
category: interface-controls
keys:
  - Selectable
  - UndeploysInto
  - IsMobileWar
---

## Starting a band

Pressing the left button over the tactical view arms the gesture, unless the player is in repair, power or sell mode, is targeting a superweapon, or is holding a building waiting for placement. Nothing is drawn yet.

The box appears once the pointer has travelled further from the press point than the system's drag distance, measured on either axis. That distance is the one Windows reports for a drag, so it follows the display scale and the pointer accessibility settings rather than a fixed number of pixels. Coast scrolling on the right button reads the same setting and doubles it.

While the button is held, the loose corner follows the pointer and the box is drawn each frame. Dragging with the right button at the same time abandons the band and selects nothing.

## What the box takes

Releasing the button clears the current selection first, unless left Shift is held, and then offers every object whose position on screen falls inside the box. An object is taken when **all of**:

- the player owns it;
- its type is [`Selectable=yes`](/keys/selectable/);
- it is out of [limbo](/glossary/#limbo);
- it is not a structure, or it is a structure that [undeploys into a vehicle](/keys/undeploysinto/) and is neither a construction yard nor an [`IsMobileWar=yes`](/keys/ismobilewar/) one.

So a deployed artillery piece or tick tank is caught by a box drawn over it, while a construction yard, a mobile war factory and every ordinary structure are not. Only the first object taken gives its acknowledgement, however many the box caught.

The test is the object's drawing position against the box on screen, not its cell against a map region, so an object whose art overlaps the box but whose position does not is left alone.
