---
key: WindDirection
summary: The facing that smoke and gas particles drift toward.
see_also: [WindEffect, BehavesLike]
when_omitted:
  kind: value
  value: "-1"
---

The value is a facing index: `0` is north and each step of `1` adds 45 degrees clockwise, up to `7` for north-west. It picks one fixed offset (at most two leptons on each axis), and that offset is the same for every particle on the map. Nothing varies it during a match.

Two tables of eight offsets are held, and a particle's [behavior](/keys/behaveslike/#scope-particletype) settles which one it reads and how. A `Smoke` or `WeakGas` particle takes its offset unscaled, on one frame out of every `10` divided by its ParticleType's [`WindEffect`](/keys/windeffect/), and not at all while that setting is `0` or less. A `Gas` particle takes its offset multiplied by `WindEffect` on every frame, on top of its own climb and its accumulated drift. The two tables agree at every facing but `3`, where the unscaled offset is a lepton shorter. No other particle behavior reads the setting.

:::caution[An out-of-range facing drifts particles by an unrelated offset]
Neither table is range-checked before the facing indexes it, and the setting has no usable built-in value. With the key absent the stored facing is `-1`, one slot before the start of both tables, and a value above `7` reads past their other end. The offset then comes from whatever memory neighbors the table, so a particle that drifts at all is pushed an unrelated distance in an unrelated direction rather than by the step the facing would name.
:::
