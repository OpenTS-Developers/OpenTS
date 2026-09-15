---
key: VisibleLoad
summary: Parsed cargo-visibility flag that the engine never uses.
no_effect: true
when_omitted:
  kind: value
  value: "no"
---

Nothing reads the stored flag, so it cannot change how the object is drawn. What a loaded transport does show is its cargo pips, which [`PipScale`](/keys/pipscale/) and [`Passengers`](/keys/passengers/) control.
