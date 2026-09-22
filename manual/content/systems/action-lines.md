---
title: Action lines
summary: "Draws a selected object's orders as lines to its target, along its route and through its queued destinations, and the sighting laser of a firing vehicle."
category: rendering-presentation
keys:
  - UnitActionLines
  - AlwaysShowActionLines
  - MovementLineColor
  - MovementLineDashed
  - MovementLineDropShadow
  - MovementLineDropShadowColor
  - MovementLineThick
  - TargetLineColor
  - TargetLineDashed
  - TargetLineDropShadow
  - TargetLineDropShadowColor
  - TargetLineThick
  - ShowNavComQueueLines
  - NavComQueueLineColor
  - NavComQueueLineDashed
  - NavComQueueLineDropShadow
  - NavComQueueLineDropShadowColor
  - NavComQueueLineThick
  - TargetLaser
  - TargetLaserColor
  - TargetLaserDashed
  - TargetLaserDropShadow
  - TargetLaserDropShadowColor
  - TargetLaserThick
  - TargetLaserTime
related:
  - type: format
    id: ui-ini
  - type: system
    id: waypoint-paths
  - type: system
    id: target-selection
---

Action lines show a selected object's current orders on the tactical map. A target line runs to what the object is attacking, and a movement line runs to where it is going. Vehicles, infantry and aircraft draw them while they are selected and belong to a house the player controls. Structures never draw action lines, and an object with neither a target nor a destination draws none.

[`UnitActionLines=no`](/keys/unitactionlines/) in `sun.ini` turns action lines off, and the in-game game controls dialog has the same switch. It does not affect the [sighting laser](#the-sighting-laser).

## When the lines are shown

The lines appear for 25 game frames after the player selects objects or gives an order with the mouse. Clicking an object, dragging a selection box and pressing a team-selection key all count as selecting. All selected objects share this period, so each new selection or order shows every selected object's lines again.

The lines also stay up while the queue-move key, Q, is held, so every leg of a queue stays in view while it is laid out.

[`AlwaysShowActionLines=yes`](/keys/alwaysshowactionlines/) in [UI.INI](/formats/ui-ini/) keeps the lines up for as long as the object is selected.

## The lines

The target line runs from the object's firing point to the point it aims at. The firing point is where its first weapon is mounted. On an object with a turret, that point turns with the turret. When the target is a moving vehicle, the line ends ahead of it, where the shot is aimed.

The movement line runs from the object to its final destination. A vehicle whose route crosses a bridge or passes between movement zones plans that route as several legs. The line ends at the far end of the last leg, not at the end of the leg being driven.

A line that ends in a cell under a bridge ends at the height of the bridge deck, even when the object is going beneath the bridge. This applies to the movement line and to queue lines.

Queue lines continue from the end of the movement line through each queued destination, in the order the object will travel them. [`ShowNavComQueueLines=no`](/keys/shownavcomqueuelines/) in [UI.INI](/formats/ui-ini/) turns them off. They appear only while the object has a current destination. A looping queue draws a closed ring, because its current destination is also its last queued one.

Every line is clipped to the tactical view. Each end gets a small square, three pixels wide or four on a thick line. An end outside the view gets no square.

The target line, the movement line, the queue lines and the sighting laser each have a separate set of color and style keys in the `[Ingame]` section of [UI.INI](/formats/ui-ini/). A line can be dashed, thick or drop-shadowed. A thick line is two pixels high.

A drop shadow is a copy of the line drawn just below it, in the color set by the matching `DropShadowColor` key. It also puts a larger square in that color behind each end square. On a thick line, a drop shadow shrinks the end squares to three pixels.

On the action lines, dashes are four pixels on and four off, and they move along the line. Their speed follows the clock, not the game speed: one pixel every 64 milliseconds on the target line, and every 128 milliseconds on the movement and queue lines.

This example sets every `[Ingame]` key the lines read. The values are examples, not defaults.

```ini title="UI.INI"
[Ingame]
AlwaysShowActionLines=yes         ; hold the lines up for as long as the object stays selected
MovementLineColor=0,255,0
MovementLineDashed=no
MovementLineDropShadow=no
MovementLineDropShadowColor=0,0,0
MovementLineThick=no
TargetLineColor=255,255,0
TargetLineDashed=no
TargetLineDropShadow=no
TargetLineDropShadowColor=0,0,0
TargetLineThick=no
ShowNavComQueueLines=yes          ; continue the queued destinations from the end of the movement line
NavComQueueLineColor=0,255,255
NavComQueueLineDashed=no
NavComQueueLineDropShadow=no
NavComQueueLineDropShadowColor=0,0,0
NavComQueueLineThick=no
TargetLaserColor=255,0,0
TargetLaserDashed=yes
TargetLaserDropShadow=no
TargetLaserDropShadowColor=0,0,0
TargetLaserThick=no
TargetLaserTime=45
```

## The sighting laser

A vehicle whose type has [`TargetLaser=yes`](/keys/targetlaser/) draws a sighting laser from its firing point to where its shot is aimed. The laser stays for [`TargetLaserTime`](/keys/targetlasertime/) frames after each shot, and disappears sooner if the vehicle loses its target.

Only shots fired while the vehicle's house is under the player's control start the laser. Selection, `UnitActionLines` and `AlwaysShowActionLines` do not affect it.

The laser is drawn with the vehicle, not over the whole map, so objects drawn after the vehicle can cover it.

Its end squares are two pixels wide, or four when thick (three when thick and drop-shadowed). Its dashes are one pixel on and one off, and they swap places every game frame.
