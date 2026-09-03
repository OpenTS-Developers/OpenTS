---
key: StartColor2
summary: The other end of the color range a spark or railgun particle is created at.
see_also: ["StartColor1", "ColorList", "ColorSpeed"]
when_omitted:
  kind: value
  value: 0,0,0
---

The second end of the range described by [`StartColor1`](/keys/startcolor1/), which covers how the two are read, how the starting color is picked between them, and what leaving both black does. Setting only one of the pair narrows the range to the span between that color and black, since the unset end stays at `0,0,0`.

:::note[A partial triplet reads as the default]
`StartColor2=255` names one channel where three are needed, so the far end of the range keeps its default color and the debug log records the line; [INI syntax](/formats/ini-syntax/#malformed-values) has the rule.
:::
