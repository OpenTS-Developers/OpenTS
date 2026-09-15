---
key: InfDeath
summary: The death sequence an infantryman killed by the warhead performs.
see_also: [Doggie, DeadBodies, InfantryExplode, FlamingInfantry, "system:laser-fences"]
when_omitted:
  kind: value
  value: "0"
---

The setting is one of six numbers, and the table gives the death each one produces. What separates the rows is whether the soldier plays a sequence where it stands or is simply taken away with an animation left in its place, which is what the two paragraphs after the table turn on.

| Value | Result |
| --- | --- |
| `0` | The soldier is removed at once, with no death sequence and no corpse |
| `1` | The gun death sequence |
| `2` | The explosion death sequence |
| `3` | The [`InfantryExplode`](/keys/infantryexplode/) animation is left where the soldier stood, and the soldier is removed |
| `4` | A [`Doggie=yes`](/keys/doggie/) type plays its own burning death; any other type is removed and leaves the [`FlamingInfantry`](/keys/flaminginfantry/) animation behind |
| `5` | A `Doggie=yes` type plays its own burning death; any other type is removed and leaves the electrocution animation behind |

Any value outside that range behaves as `0`.

```ini title="rules.ini"
[MyFlameWH] ; example WarheadType
InfDeath=4
```

A sequence played on the soldier itself (`1`, `2`, and the dog branch of `4` and `5`) holds the body in place until the animation runs out. The soldier is removed at that point, and every type but a dog drops one of the [`DeadBodies`](/keys/deadbodies/) corpses as it goes.

Three deaths never reach the setting at all. A prone cyborg and a jump-jet infantryman both leave the `InfantryExplode` animation, and a soldier that dies over water after being knocked off a height leaves a wake and a splash. A soldier killed by a [laser fence](/systems/laser-fences/) is forced onto `5` whatever its attacker's warhead says. A cyborg killed by forced damage is removed at once whatever the setting names: the sequence for `1` or `2` still starts, but the body is taken away on the same step.

:::caution[The electrocution animation is fixed to a slot]
`InfDeath=5` does not name its animation. It takes the second animation registered in the [`[Animations]` list](/formats/rules-registries/), which is `ELECTRO` in the shipped rules, so inserting, removing or reordering entries ahead of that position changes what an electrocuted soldier leaves behind.
:::

:::caution[A burning soldier is drawn in the observer's own colors]
The `FlamingInfantry` animation of `InfDeath=4` is recolored with the local player's color scheme rather than with the dead soldier's house colors, so the same death looks different on each machine in the game.
:::
