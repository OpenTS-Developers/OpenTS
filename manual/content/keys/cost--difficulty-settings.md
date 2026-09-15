---
key: Cost
scope: difficulty-settings
label: Difficulty price multiplier
see_also: ["system:difficulty"]
when_omitted:
  kind: value
  value: "1"
  note: The difficulty block is re-read from fixed defaults whenever its section is present, so a later file that contains the section without this key restores 1 rather than keeping the earlier value.
---

`[Easy]`, `[Normal]` and `[Difficult]` each set their own multiplier, and a house takes the one for [the difficulty slot it is assigned](/systems/difficulty/#from-the-setting-to-a-slot). Every price that house pays is [the object's own `Cost=`](/keys/cost/#scope-aircrafttype) multiplied by this value, so a figure above 1 pays more. It is the balance a factory charges out over its production steps. The same multiplied price is what a computer house tests its credits against before it commits to anything.

The multiplier changes prices alone. Build time is a separate axis, set by [`BuildTime=`](/keys/buildtime/#scope-difficulty-settings) in the same section.

```ini title="rules.ini"
[Difficult]
Cost=1.2
```

The product is worked out once, when the house is given its slot, alongside the [country's own multiplier](/keys/cost/#scope-housetype) outside campaign games. In a campaign game the country's figure is dropped and this one stands alone.
