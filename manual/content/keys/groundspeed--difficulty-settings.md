---
key: Groundspeed
scope: difficulty-settings
label: Difficulty speed multiplier
see_also: ["system:difficulty"]
when_omitted:
  kind: value
  value: "1"
  note: The difficulty block is re-read from fixed defaults whenever its section is present, so a later file that contains the section without this key restores 1 rather than keeping the earlier value.
---

`[Easy]`, `[Normal]` and `[Difficult]` each set their own multiplier, and a house takes the one for [the difficulty slot it is assigned](/systems/difficulty/#from-the-setting-to-a-slot). Every object of that house has its top speed multiplied by it before its own veterancy bonus and current throttle are applied, so a figure above 1 moves faster.

The multiplier reaches the driving, walking, hovering, mechanical and tunneling locomotors. An aircraft flying on its own locomotor and an object in the air on a jumpjet take their speed from elsewhere, and the multiplier does not scale either.

The product is worked out once, when the house is given its slot, alongside the [country's own multiplier](/keys/groundspeed/#scope-housetype) outside campaign games and [`GameSpeedBias`](/keys/gamespeedbias/) everywhere.

```ini title="rules.ini"
[Difficult]
Groundspeed=1.2
```
