---
title: Drop pods
summary: "Delivers infantry through the GDI Drop Pods superweapon or an infantry-only TeamType."
category: superweapons-special
keys:
  - AtmosphereEntry
  - C4Warhead
  - DropPod
  - Droppod
  - DropPodAngle
  - DropPodHeight
  - DropPodInfantryMinimum
  - DropPodInfantryMaximum
  - DropPodPuff
  - DropPodSpeed
  - DropPodWeapon
related:
  - type: internal
    id: locomotion
---

## Entry paths

### Drop Pods superweapon

The superweapon drops a squad of elite infantry around its target cell. The squad size is a random whole number from [`DropPodInfantryMinimum`](/keys/droppodinfantryminimum/) to [`DropPodInfantryMaximum`](/keys/droppodinfantrymaximum/), both included. Each soldier is an `E1` or an `E2` with equal chance. These two type names are fixed.

The squad lands in a cluster. The first soldier is aimed at the enterable cell nearest the target. Each later soldier is aimed near an enterable cell next to the previous soldier's destination.

The squad can arrive smaller than the chosen size. The superweapon allows three placement attempts per soldier, pooled across the squad, so a squad of four gets twelve attempts. An attempt fails when the soldier cannot enter the chosen cell or cannot be placed at the start of its fall. Each attempt creates a new soldier and discards it if the attempt fails. When a soldier cannot enter its chosen cell, the aim point stays where it was, so the next attempt starts from the same place. One blocked spot can therefore use up the rest of the pool.

### Droppod TeamType

The other path is a reinforcement team. A **TeamType** is the INI definition a team is built from, and it names a **TaskForce**, the roster of object types and counts the team is filled with. [TeamTypes and AI triggers in brief](/systems/ai-team-production/#teamtypes-and-ai-triggers-in-brief) introduces both and the sections that declare them.

[`Droppod=yes`](/keys/droppod-teamtype/) on the TeamType makes the team arrive by pod. It applies only when the team is delivered by the [Reinforcement (team)](/mapping/actions/taction-reinforcements/) or [Reinforcement (team) [at waypoint]](/mapping/actions/taction-reinforcements-special/) trigger action. Teams the AI builds and recruits never arrive by pod.

```ini title="AI.INI, AIFS.INI, or map file"
[TaskForces]
0=MyInfantryTaskForce

[MyInfantryTaskForce] ; example TaskForce
Name=Drop infantry
0=2,E1
1=1,E2

[TeamTypes]
0=MyDropTeam

[MyDropTeam] ; example TeamType
Name=Drop team
TaskForce=MyInfantryTaskForce
Droppod=yes
```

Every TaskForce member must be an InfantryType. If any member is another kind of type, such as a vehicle or aircraft, the whole team arrives as an ordinary reinforcement. Its infantry do not drop.

The first member lands at the nearest usable cell to the team's waypoint, or to the waypoint the action names. Each later member is aimed at the cell north of the previous member's cell. When that cell is outside the playfield, the next neighbor clockwise is used instead. Those later cells are not checked for room, so a member aimed at a blocked cell can be destroyed on landing, as [Touchdown](#touchdown) describes.

## Approach and descent

For the fall, each passenger gets a temporary drop-pod locomotor. Its usual locomotor is set aside through [piggybacking](/internals/locomotion/#piggybacking) and restored at touchdown.

### Approach selection

A pod starts above and to one side of its destination, then falls in a straight line onto it. The start point is [`DropPodHeight`](/keys/droppodheight/) above the ground at the destination, and `DropPodHeight / tan(DropPodAngle)` away horizontally.

The engine tries four approaches in this order and uses the first whose start point lies over the [playable area](/glossary/#playable-area):

1. NE, offset along `+X`;
2. NW, offset along `-X`;
3. SE, offset along `+Y`;
4. SW, offset along `-Y`.

If the first three start points are all outside the playable area, the pod uses SW, even when that start point is outside it as well.

The approach decides where the [`AtmosphereEntry`](/keys/atmosphereentry/) animation appears, since it is created at the start point. It also decides how the falling pod looks and which landing animation plays:

| Approach | Start offset | `POD.SHP` frame while falling | [`DropPod`](/keys/droppod-global-rules/) entry with four entries |
| --- | --- | ---: | ---: |
| NE | `+X` | 0 | 0 |
| NW | `-X` | 1 | 1 |
| SE | `+Y` | 0 | 2 |
| SW | `-Y` | 1 | 3 |

While it falls, the passenger is drawn as a frame of `POD.SHP` in place of its own art. The file name is fixed. Only frames 0 and 1 are used: NE and SE show frame 0, and NW and SW show frame 1.

Number the approaches 0 to 3 in the order above. The landing animation is the `DropPod` entry at that number, wrapping back to the first entry past the end of the list. A four-entry list gives each approach its own animation. With two entries, NE and SE share the first and NW and SW share the second. A single entry serves every approach.

### Descent and airborne effects

```ini title="rules.ini"
[General]
DropPodHeight=1500   ; example values; the key pages give the defaults
DropPodSpeed=40
DropPodAngle=0.785398
```

[`DropPodAngle`](/keys/droppodangle/) is the fall's angle above the horizontal, in radians. The engine clamps it to 22.5 through 67.5 degrees. A larger angle gives a steeper fall that starts closer to the destination.

Each frame the pod travels `speed` along its line, where `speed` is the larger of [`DropPodSpeed`](/keys/droppodspeed/) and `height above ground / 10 + 2`. A pod high above the ground therefore falls fast and slows to `DropPodSpeed` near the ground. Of that distance, `cos(DropPodAngle) * speed` is horizontal and `sin(DropPodAngle) * speed` is vertical.

The `AtmosphereEntry` animation appears only if the passenger is placed at the start point on the first try. The engine tries once more when that placement fails, but creates no animation on the second try.

[`DropPodWeapon`](/keys/droppodweapon/) adds two effects while the pod is in the air:

- Every six frames, the `SMOKEY` animation, a fixed name, appears at the pod's position.
- Every three frames, the pod fires on its destination unless the object standing there is allied to the passenger's owner. An empty destination cell is fired on as well. When several objects share the cell, only one of them is checked.

Each shot lands one third of a cell from the destination, in a random direction. It explodes for twice the weapon's `Damage`, using the weapon's `Warhead`, with the passenger as the attacker. The weapon's `Report` sound plays at the pod, and the warhead's clear-ground explosion animation for that damage plays at the impact point.

Allied objects near the impact point still take damage. The ally check covers only the one object tested in the destination cell.

No projectile is fired, and the weapon's range and rate of fire have no effect. Without a `DropPodWeapon`, the pod fires nothing and leaves no `SMOKEY` trail.

## Touchdown

When the pod reaches the ground, the passenger's usual locomotor returns and the engine places the passenger where the pod landed. A placed passenger gets the landing animation its approach selected, goes idle, and scatters from the landing point.

:::danger[Configure the pod animations]
Give `DropPod` at least one entry. If the list is absent or empty, the game crashes the first time a passenger is placed at touchdown.

Also set `AtmosphereEntry`. If it is unset, the game crashes the first time a passenger is placed at the start of its fall.

Give `DropPodWeapon` a nonzero `Damage` and a warhead that lists at least one `AnimList` animation. Otherwise the game crashes on the pod's first shot.

Give [`C4Warhead`](/keys/c4warhead/) at least one `AnimList` animation. If it is unset or lists none, the game crashes at the first blocked touchdown.
:::

:::caution[A blocked touchdown destroys the passenger]
If the passenger cannot be placed where the pod lands, it is destroyed. The landing point takes an explosion of 100 damage with [`C4Warhead`](/keys/c4warhead/), with the passenger as the attacker. The explosion can damage objects within 1.5 cells, and the actual loss depends on distance, armor, and the warhead. The warhead's clear-ground animation plays, but the `DropPod` landing animation does not.
:::

:::caution[Preserve the exact key spelling]
`Droppod` in a TeamType section and `DropPod` in `[AudioVisual]` are distinct keys; the [`Droppod` key page](/keys/droppod-teamtype/) states the spelling rule.
:::
