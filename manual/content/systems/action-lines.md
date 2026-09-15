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

Every rendered frame, each selected vehicle, infantryman or aircraft that belongs to a house the player controls draws lines for the orders it is running. A target line runs from its firing point to what it is attacking, and a movement line runs from where it stands to the end of its route. Structures draw none, whatever they are doing. `UnitActionLines=no` in `sun.ini`, which the game controls dialog also offers, switches all of them off.

## When the lines are shown

The lines appear for 25 frames after the player selects something or gives an order, and a new selection or order restarts that time for every selected object at once. They also stay up while the queue-move key, Q, is held, so a queue can be laid out with every leg in view. [`AlwaysShowActionLines=yes`](/keys/alwaysshowactionlines/) in [UI.INI](/formats/ui-ini/) keeps them up for as long as the object is selected.

## The lines

The target line runs from the firing point, the turret of a turreted vehicle, to where the shot is aimed, which follows a moving target. The movement line runs from the object to the far end of its planned route. A route that crosses a bridge or passes between movement zones is planned as several legs, so the line reaches the end of the last leg rather than the one being walked. An end that lies under a bridge is drawn on the deck.

With [`ShowNavComQueueLines=yes`](/keys/shownavcomqueuelines/) in [UI.INI](/formats/ui-ini/) the queued destinations continue from the end of the movement line, one leg per queued order in the order they will be traveled. A queue that loops draws a closed ring, since its current destination is also its last.

Each line is clipped to the tactical view and draws a small square on each end; an end outside the view draws no square. The three lines and the sighting laser each take their own color from [UI.INI](/formats/ui-ini/) `[Ingame]`, which also sets whether a line is dashed, thick or shadowed, and holds `TargetLaserTime`. Each can be drawn dashed, drawn thick as two rows of pixels, or over a shadow drawn below it. Dashes are four pixels on and four off. They move along the line by the clock rather than by game frame: one pixel every 64 milliseconds on the target line, and every 128 milliseconds on the others.

Every switch and style setting the lines read, in the one section that holds them:

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

A vehicle whose type has [`TargetLaser=yes`](/keys/targetlaser/) draws a separate line from its firing point to where its shot is aimed, for [`TargetLaserTime`](/keys/targetlasertime/) frames after each shot. The line appears only while its house is under the player's control. It is drawn along with the vehicle itself rather than over the whole map, so objects drawn after the vehicle can cover it. Its end squares are two pixels, and its dashes are one pixel on and one off, stepping seven pixels each frame.
