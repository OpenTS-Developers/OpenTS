---
key: Normalized
scope: animtype
label: Animation rate
see_also: ["Rate", "RandomRate"]
when_omitted:
  kind: value
  value: "no"
---

The animation's frame delay is rescaled against the game speed setting when the animation starts. It keeps roughly the same rate on the clock however fast or slow the match is running. The rescaling is applied last: a [`RandomRate`](/keys/randomrate/) pick is made first, and whichever delay comes out of it is what gets rescaled.
