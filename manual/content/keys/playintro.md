---
key: PlayIntro
summary: Controls the one-time `EVA.VQA` startup movie.
when_omitted:
  kind: value
  value: "yes"
---

The entry sits in the `[Intro]` section of `sun.ini`. `PlayIntro=yes` selects the
first-time startup path. The game writes `PlayIntro=no` back to `sun.ini` before
playing `EVA.VQA`, so a later start skips that movie unless the setting is enabled
again.

[`FROMINSTALL`](/using/command-line/from-install/) selects the same path without
reading this setting. The other startup movies are not gated by `PlayIntro`.
