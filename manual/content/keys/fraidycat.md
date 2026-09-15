---
key: Fraidycat
summary: Sends the soldier to panic at the first hit and makes it break off and run rather than fight.
see_also: [Fearless, Doggie, Ammo, Scatter]
when_omitted:
  kind: value
  value: "no"
---

Fear is a figure from `0` to `255` that a soldier accumulates as it is shot at. Four thresholds along it matter: `10` is where a soldier goes to ground, `100` is what a hit from a source normally raises it to, `200` is panic, and `255` is the ceiling. A `Fraidycat=yes` soldier goes to `200` on any hit from a source that lands while its fear is still under `100`. It goes to `255` the moment it runs out of [`Ammo`](/keys/ammo/), and that also drops an Attack or Hunt mission back to Guard. An armed civilian therefore stops fighting when its clip is empty and refills it as its fear works all the way back to zero.

Above `10` fear, a soldier of such a type that is standing still with nowhere to go scatters. That happens on every logic frame while the condition holds, and again in place of the idle animation a computer-owned soldier would otherwise have played. A computer-owned one wanders off on one idle in eleven even at no fear at all. The scatter itself is stronger than an ordinary one. Any other soldier refuses to scatter while it holds a target; one of these does not, so it abandons the shot and moves.

:::caution[A prone one does not get up when re-ordered]
Ordering a prone soldier of a player-controlled house to the same destination twice normally makes it stand and run. A `Fraidycat=yes` soldier is excluded from that, as a [`Cyborg=yes`](/keys/cyborg/) one is.
:::

:::caution[The panic ignores `Fearless`]
The jump to `200`, and the jump to `255` on running out of ammunition, are the only two paths into fear that do not test [`Fearless=yes`](/keys/fearless/) first. Since the recovery step and the [Unpanic](/mapping/missions/tmission-unpanic/) mission both do test it, a type declaring `Fraidycat=yes` and `Fearless=yes` together is permanently panicked from its first hit onward.
:::
