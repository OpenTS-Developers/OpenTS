---
key: WebDuration
summary: The number of frames a webbed infantryman stays pinned.
see_also: [Webby, WebDurationVariation, IsWebImmune, "system:target-selection"]
when_omitted:
  kind: value
  value: "300"
---

Frames run fifteen to the second, so the built-in `300` pins a soldier for twenty seconds. The figure is the midpoint of the range [`WebDurationVariation`](/keys/webdurationvariation/) spreads around it, not a ceiling.

```ini title="rules.ini"
[MyWebWH] ; example WarheadType
Webby=yes
Particle=MyWebSys ; example ParticleSystemType
WebDuration=600 ; forty seconds, give or take the variation
```

While the count runs the soldier struggles in place: it cannot walk, and pointing it at a destination shows the no-move cursor. When the count reaches zero the soldier drops back to prone and carries on. A second web only ever lengthens the wait. The new figure replaces the remaining one when it is longer and is discarded when it is shorter, so webbing an already-webbed soldier can never free it early.

A quarter of this figure is the cutoff an object with a web weapon uses when it decides whether a soldier is worth webbing again; [target selection](/systems/target-selection/#which-weapon-the-score-assumes) covers that choice.

The setting is read only while [`Webby=yes`](/keys/webby/) stands in the same section.
