---
key: WebDurationVariation
summary: The number of frames the web duration is shifted at random in either direction.
see_also: [Webby, WebDuration]
when_omitted:
  kind: value
  value: "25"
---

Each webbed soldier draws a whole number between minus and plus this figure, inclusive, and that shift is added to [`WebDuration`](/keys/webduration/). The built-in `25` therefore spreads a web across a range of just over three seconds, which is what keeps a group of soldiers caught by one shot from breaking loose together. A figure of `0` gives every soldier the same wait.

```ini title="rules.ini"
[MyWebWH] ; example WarheadType
Webby=yes
Particle=MyWebSys ; example ParticleSystemType
WebDuration=600
WebDurationVariation=90 ; each soldier waits 34 to 46 seconds
```

The sign makes no difference: the two ends of the range are put in order before the draw, so a negative figure behaves exactly as its positive counterpart.

The setting is read only while [`Webby=yes`](/keys/webby/) stands in the same section.

:::caution[A variation larger than the duration can pin nothing]
The shift is applied without a floor. A draw that takes the total to zero or below leaves the soldier's remaining wait untouched, so it plays one struggle and stands straight back up, still having taken no damage from the hit.
:::
