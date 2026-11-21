---
key: DestroyableBridges
summary: Whether a wall-destroying blast can bring a bridge span down.
see_also: [BridgeStrength, IonCannonWarhead, C4Warhead]
when_omitted:
  kind: value
  value: "yes"
  note: Each scenario starts with this built-in default.
---

With `DestroyableBridges=no`, no blast can damage a bridge span. With `yes`, a blast from a warhead with [`Wall=yes`](/keys/wall/#scope-warheadtype) can damage the span in its cell. [`BridgeStrength`](/keys/bridgestrength/) sets the chance that each such blast succeeds, and the ion cannon's warhead skips that roll.

Two trigger actions damage a bridge span whatever this key says: [Destroy attached building](/mapping/actions/taction-destroy-object/) and [Apply 100 damage at...](/mapping/actions/taction-damage/).

:::caution[The entry is read in campaigns only]
Only a single-player mission reads `[SpecialFlags]` from the map. In every other game type, the [`BridgeDestruction`](/keys/bridgedestruction/) option decides whether bridges can be destroyed, whatever the map says.
:::
