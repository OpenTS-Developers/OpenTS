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

The superweapon chooses an inclusive count between [`DropPodInfantryMinimum`](/keys/droppodinfantryminimum/) and [`DropPodInfantryMaximum`](/keys/droppodinfantrymaximum/). Each requested passenger is an elite `E1` or `E2`, selected with equal probability.

The placement loop shares one budget of `3 * count` attempts across the whole squad. Every attempt consumes the budget, successful or not, so one hard-to-place passenger can use up attempts that other passengers would otherwise receive. The delivered count is lower when the budget runs out before enough nearby cells an infantryman can enter are found.

### Droppod TeamType

The other path is a reinforcement team. A **TeamType** is the INI definition a team is built from, and it names a **TaskForce**, the roster of object types and counts the team is filled with. [TeamTypes and AI triggers in brief](/systems/ai-team-production/#teamtypes-and-ai-triggers-in-brief) introduces both and the sections that declare them. [`Droppod=yes`](/keys/droppod-teamtype/) on the TeamType is what makes the reinforcement arrive by pod instead of on foot.

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

Every TaskForce member must resolve to an InfantryType. If any member resolves to a vehicle or aircraft type, reinforcement creation uses the configured waypoint or map-edge path instead; it does not drop the infantry subset.

## Approach and descent

Both entry paths attach a temporary drop-pod locomotor. The passenger's current locomotor is retained through [piggybacking](/internals/locomotion/#piggybacking) and restored at touchdown.

### Approach selection

The horizontal start offset is `DropPodHeight / tan(DropPodAngle)`. The engine checks candidates in this order:

1. NE at `+X`;
2. NW at `-X`;
3. SE at `+Y`; then
4. SW at `-Y` as an unconditional fallback.

The first three elevated start coordinates must lie inside the [playable area](/glossary/#playable-area). The SW coordinate is selected when all three checks fail, even when it is also outside that area.

The direction settled here decides four things at once. It fixes which way the pod comes in, and so the sign of the horizontal step taken on every frame of the descent. The same direction picks one of the two frames in the hard-coded `POD.SHP` artwork, which is the pod's appearance for the whole fall. It picks the entry in [`DropPod`](/keys/droppod-global-rules/) that supplies the landing animation at touchdown. And it fixes the coordinate the [`AtmosphereEntry`](/keys/atmosphereentry/) animation is created at, which is the elevated start coordinate the pod is placed on.

The table sets the four directions against those choices. The pod frames repeat: NE and SE share one, NW and SW share the other, so the artwork distinguishes only the axis the pod is falling along. The landing slots do not repeat, so a four-entry `DropPod` list gives all four approaches distinct touchdown animations.

| Direction | Offset axis | Hard-coded `POD.SHP` frame | `DropPod` landing slot with four entries | `AtmosphereEntry` effect |
| --- | --- | ---: | ---: | --- |
| NE | `+X` | 0 | 0 | Elevated NE start coordinate |
| NW | `-X` | 1 | 1 | Elevated NW start coordinate |
| SE | `+Y` | 0 | 2 | Elevated SE start coordinate |
| SW | `-Y` | 1 | 3 | Elevated SW start coordinate |

### Descent and airborne effects

```ini title="rules.ini"
[General]
DropPodHeight=1500
DropPodSpeed=40
DropPodAngle=0.785398
```

[`DropPodAngle`](/keys/droppodangle/) is in radians and is clamped to 22.5 through 67.5 degrees. Per-frame speed is the greater of [`DropPodSpeed`](/keys/droppodspeed/) and `height above ground / 10 + 2`. The horizontal component is `cos(DropPodAngle) * speed`; descent is `sin(DropPodAngle) * speed`.

The hard-coded `POD.SHP` frame in the table is used while the passenger is airborne. When the first elevated placement succeeds, the engine also creates the configured `AtmosphereEntry` animation at that coordinate. A failed first placement is repeated once without creating this effect.

The [`DropPodWeapon`](/keys/droppodweapon/) branch controls both airborne effects:

- Every six frames, it creates the hard-coded `SMOKEY` animation at the pod's trail position.
- Every three frames, it reads whatever vehicle, infantryman, aircraft or structure stands in the destination cell, and fires unless that object is allied to the passenger's house. A cell nobody is standing in receives covering fire all the same.
- The impact coordinate is scattered within radius 85 of the destination. The passenger is the source of raw `2 * Damage` applied with the weapon's warhead. The weapon report plays, and the matching clear-land combat animation is created at the impact coordinate.

The whole `DropPodWeapon` branch applies area damage directly: no projectile is created, and neither the weapon's range nor its rate of fire is read. A null `DropPodWeapon` skips the entire branch, including `SMOKEY`.

## Touchdown

At ground contact, the passenger enters [limbo](/glossary/#limbo), the previous locomotor replaces the piggyback locomotor, and the engine attempts to place the passenger at the current ground-contact coordinate. Successful placement creates the landing animation selected by the approach direction, enters idle behavior, and scatters the passenger.

:::danger[Provide at least one landing animation]
The landing branch selects from `DropPod` using `direction % list length`. An absent or empty list makes the selection a division by zero, and the game crashes as the first pod touches down. Four entries give NE, NW, SE, and SW distinct landing slots as shown above.
:::

:::caution[A blocked touchdown destroys the passenger]
If the restored locomotor cannot place the passenger, the engine applies raw damage 100 at the current ground-contact coordinate with the passenger as the source and [`C4Warhead`](/keys/c4warhead/) as the warhead. Standard 1.5-cell explosion processing can damage nearby objects, and a clear-land combat animation is selected from the same damage and warhead. The passenger is then deleted; the configured landing animation, idle transition, and scatter do not run. Actual HP loss depends on distance, armor, and the warhead's behavior.
:::

:::caution[Preserve the exact key spelling]
`Droppod` in a TeamType section and `DropPod` in `[AudioVisual]` are distinct keys; the [`Droppod` key page](/keys/droppod-teamtype/) states the spelling rule.
:::
