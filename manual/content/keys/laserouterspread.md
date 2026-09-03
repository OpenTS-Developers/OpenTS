---
key: LaserOuterSpread
summary: How far each channel of a laser beam's glow may wander from its color, redrawn every frame.
see_also: ["IsLaser", "LaserOuterColor", "LaserInnerColor"]
when_omitted:
  kind: value
  value: "0,0,0"
---

Three channel figures, each the largest step the matching channel of [`LaserOuterColor`](/keys/laseroutercolor/) may take in either direction. A fresh offset is picked for every channel on every frame the beam is drawn and the result is held to the 0-to-255 range, so the glow shimmers over the beam's life instead of sitting at one flat color. The offset applies to the whole glow at once, not per pixel, and the beam's core is never touched by it.

```ini title="rules.ini"
[MyObeliskRay] ; example WeaponType
IsLaser=yes
LaserOuterColor=128,0,0
LaserOuterSpread=40,0,0 ; the glow's red wanders between 88 and 168
```

Since the glow is drawn by blending its red alone at normal detail, only the red figure has a visible result there; the green and blue figures shimmer a color that is never put on the screen. All three show at the lowest detail setting, which draws the glow flat in its full color. A glow suppressed by a `LaserOuterColor` of `0,0,0` is not brought back by any spread.

The figures are read from the weapon in the object's first slot whichever slot fired.

:::note[A partial triplet reads as the default]
`LaserOuterSpread=40` names one channel where three are needed, so the spread keeps its default figures and the debug log records the line; [INI syntax](/formats/ini-syntax/#malformed-values) has the rule.
:::
