---
key: LaserOuterColor
summary: The color of the pair of glow lines drawn alongside a laser beam's core.
see_also: ["IsLaser", "LaserInnerColor", "LaserOuterSpread"]
when_omitted:
  kind: value
  value: "0,0,0"
---

Two extra lines are drawn a pixel away from the beam's core, both in this color, and both jittered each frame by [`LaserOuterSpread`](/keys/laserouterspread/). A value of exactly `0,0,0` suppresses them: the color is compared against black before the jitter is applied, so a spread cannot bring back a glow that was left at black.

```ini title="rules.ini"
[MyObeliskRay] ; example WeaponType
IsLaser=yes
LaserInnerColor=255,0,0
LaserOuterColor=128,32,32 ; only the 128 reaches the screen at normal detail
```

:::caution[Only the red channel of the glow is drawn]
The two glow lines blend their red onto whatever lies beneath them and leave the green and blue of the pixel exactly as they were, so the green and blue written here change nothing. The one exception is the lowest detail setting, which draws both lines as flat lines in the full color instead of blending them.
:::

The color is read from the weapon in the object's first slot whichever slot fired.

:::note[A partial triplet reads as the default]
`LaserOuterColor=128` names one channel where three are needed, so the glow keeps its default color and the debug log records the line; [INI syntax](/formats/ini-syntax/#malformed-values) has the rule.
:::
