---
key: RadialColor
summary: The color of the radius ring a selected structure draws on the tactical map.
see_also: [HasRadialIndicator, "system:cloaking"]
---

Three comma-separated channel values from `0` to `255`.

:::note[A partial triplet reads as the default]
`RadialColor=255` names one channel where three are needed, so the ring keeps its default color and the debug log records the line; [INI syntax](/formats/ini-syntax/#malformed-values) has the rule.
:::
