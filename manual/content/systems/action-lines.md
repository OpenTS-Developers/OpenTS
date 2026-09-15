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

Every rendered frame, each selected vehicle, infantryman or aircraft belonging to a house the player controls draws lines for the orders it is carrying: a target line from its firing point to what it is attacking, a movement line from where it stands to the end of its route, and the destinations queued behind that. Structures draw none, whatever they are doing. `UnitActionLines=no` in `sun.ini`, which the game controls dialog also offers, switches all of them off.

## When the lines are shown

The lines appear for 25 frames after the player selects something or gives an order, and a new selection or order restarts that time for every selected object at once. They also stay up while the queue-move key, Q, is held, so a queue can be laid out with every leg in view, and [`AlwaysShowActionLines=yes`](/keys/alwaysshowactionlines/) keeps them up for as long as the object is selected.

## The lines

The target line runs from the firing point, the turret of a turreted vehicle, to where the shot is aimed, which follows a moving target. The movement line runs from the object to the far end of its planned route: a route that crosses a bridge or passes between movement zones is planned as several legs, and the line reaches the end of the last leg rather than the one being walked. An end that lies under a bridge is drawn on the deck.

With [`ShowNavComQueueLines=yes`](/keys/shownavcomqueuelines/) the queued destinations continue from the end of the movement line, one leg per queued order in the order they will be traveled. A queue that loops draws a closed ring, since its current destination is also its last.

Each line is clipped to the tactical view and carries a small square on each end; an end outside the view draws no square. The three lines and the sighting laser each take their own color from [UI.INI](/formats/ui-ini/), and each can be drawn dashed, doubled to two rows, or over a shadow drawn below it. Dashes are four pixels on and four off and move along the line by the clock rather than by game frame: one pixel every 64 milliseconds on the target line and every 128 on the others.

## The sighting laser

A vehicle whose type has [`TargetLaser=yes`](/keys/targetlaser/) draws a separate line from its firing point to where its shot is aimed for [`TargetLaserTime`](/keys/targetlasertime/) frames after each shot, only while its house is under the player's control. It is drawn along with the vehicle itself rather than over the whole map, so objects drawn after the vehicle can cover it. Its end squares are two pixels, and its dashes are one pixel on and one off, stepping seven pixels each frame.
