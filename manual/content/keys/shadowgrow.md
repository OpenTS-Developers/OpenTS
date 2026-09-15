---
key: ShadowGrow
summary: Parsed multiplayer default that the engine never uses.
no_effect: true
see_also: [ShroudGrow, ShroudRate]
when_omitted:
  kind: value
  value: "yes"
---

The value is stored with the other multiplayer defaults and never read; no session option is seeded from it and no setup screen offers one. The spelling that makes the shroud creep back is [`ShroudGrow`](/keys/shroudgrow/) in `[AudioVisual]`, on the interval [`ShroudRate`](/keys/shroudrate/) sets, and it applies to every session type alike.
