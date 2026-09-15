---
key: Gravity
summary: The downward pull applied each frame to ballistic shots, hover cushions and particles.
see_also: [Arcing, Floater, HoverHeight, HoverDampen, Elasticity]
when_omitted:
  kind: value
  value: "3"
---

```ini title="rules.ini"
[AudioVisual]
Gravity=6
```

The figure is subtracted from a falling object's vertical speed once per game frame, so it is an acceleration measured in leptons rather than a distance. Three unrelated systems read it, and one of them divides it in whole numbers.

- **Ballistic shots.** A projectile in flight loses this much vertical speed each frame. A [`Floater=yes`](/keys/floater/) projectile loses half as much, which is what gives it its drifting arc. The same figure gates the shot before it is taken. An [`Arcing=yes`](/keys/arcing/) weapon reports a target it has no ballistic solution against as out of range, however close it stands. A firing structure solves its launch speed and barrel pitch from the figure too. Every weapon that throws an unguided projectile also gets its launch speed solved from this figure once, as the rules are read. Changing it therefore retunes the reach of every such weapon in the game.
- **Hover cushions.** A hovering or levitating object balances on a cushion of altitude measured in leptons. It loses this much of the cushion every frame and pushes back in proportion once it has sunk below [`HoverHeight`](/keys/hoverheight/), with [`HoverDampen`](/keys/hoverdampen/) settling the result. Below a quarter of that height it is given one extra shove of a third of this figure, and because the setting is stored as a whole number, so is the third. With the `6` the shipped rules assign, the shove is `2`; a figure of `3` to `5` gives `1`; `2` or less gives nothing at all, so a very light gravity loses the extra shove entirely.
- **Particles.** A gas or weak-gas particle has its velocity rebuilt from nothing every other frame: two leptons plus this figure, straight down. The setting is the whole of such a cloud's fall, and nothing it inherited from its launch survives. The two share one routine, so a weak-gas particle takes the pull on the same terms. A spark is pulled down by it too, with the qualification below.

Thrown debris and bouncing animations do not read the setting. They fall at a fixed rate of their own, so raising or lowering this figure leaves wreckage and bouncing shells behaving exactly as they did.

:::caution[Sparks fall faster than the figure states]
A spark's step is worked out by copying its running velocity, which has already had this frame's pull taken out of it, and then subtracting the figure from that copy a second time. The running velocity still builds at the stated rate, but every step the spark actually takes is one further pull short of it. A spark shower therefore sinks faster than anything else the setting governs, and the gap grows with the value.
:::
