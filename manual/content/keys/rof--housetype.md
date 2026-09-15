---
key: ROF
scope: housetype
label: Country reload multiplier
see_also: ["system:difficulty"]
when_omitted:
  kind: value
  value: "1.0"
---

The delay an object of a house of this country waits before its next shot is [the firing weapon's own `ROF=`](/keys/rof/#scope-weapontype) multiplied by this value. It multiplies the delay rather than the rate, so a figure above 1 fires more slowly. The product is worked out once, when the house is [given its difficulty slot](/systems/difficulty/#how-the-figures-are-combined), and not per shot.

```ini title="rules.ini"
[NOD]
ROF=1.25 ; NOD objects wait 25% longer between shots
```

A campaign game drops the country's contribution, so this value shapes skirmish and multiplayer games only; the [difficulty setting's own multiplier](/keys/rof/#scope-difficulty-settings) applies in both.
