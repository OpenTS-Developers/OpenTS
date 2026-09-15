---
key: Next
summary: The animation this one turns into instead of finishing.
see_also: ["LoopCount", "Start", "Surface", "Rate", "End"]
when_omitted:
  kind: value
  value: "none"
---

When the last pass finishes, the animation does not end: it adopts the named type in place and plays on. Nothing new is created and nothing is removed, so an explosion that settles into a column of smoke is one object from start to finish rather than two.

The named type's stage count, loop count, frame delay and rate range all take over at that moment. Its [`Report=`](/keys/report/#scope-animtype) sound is played as though the animation had just begun. The chain is followed one type at a time, each time the animation reaches an end. A pair of types naming each other therefore keeps one animation alive indefinitely rather than building anything up.

A name no `[Animations]` entry declares does not fail the read: an empty animation type is created under that name instead. An animation that chains into one of those holds no artwork and no stages, so it disappears on its next logic frame.

## What the change of type does not carry over

- A stage is the animation's frame counter, and the frame drawn is the type's start frame plus it. Chaining puts the animation on its new type's [`Start`](/keys/start/) as a stage, so a chained-to type whose start frame is not zero opens on twice that frame.
- The layer and the height are those the animation was created with, so the new type's [`Surface=`](/keys/surface/) is not read and the animation stays where the first type put it.
- [`Reverse=yes`](/keys/reverse/) on the new type is not applied; the animation keeps stepping the way it already was.
- Demand-loaded artwork is given back for the type the animation was holding when it was removed, and only where that type declares [`FreeAfterPlaying=yes`](/keys/freeafterplaying/). The earlier links in a chain therefore keep their artwork for the rest of the scenario however they are flagged.
