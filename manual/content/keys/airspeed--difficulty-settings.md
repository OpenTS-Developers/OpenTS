---
key: Airspeed
scope: difficulty-settings
label: Difficulty air-speed multiplier
see_also: ["system:difficulty", Groundspeed, Speed]
when_omitted:
  kind: value
  value: "1"
  note: The difficulty block is re-read from fixed defaults whenever its section is present, so a later file that contains the section without this key restores 1 rather than keeping the earlier value.
---

`[Easy]`, `[Normal]` and `[Difficult]` each set their own multiplier. A house folds the one belonging to [the slot it is assigned](/systems/difficulty/#from-the-setting-to-a-slot) into an air-speed figure it keeps, alongside [`GameSpeedBias`](/keys/gamespeedbias/) and, outside a campaign game, [the country's own multiplier](/keys/airspeed/#scope-housetype).

```ini title="rules.ini"
[Difficult]
Airspeed=1.2
```

An aircraft in flight covers ground at its own type's [`Speed=`](/keys/speed/#scope-aircrafttype) figure, scaled by the throttle its flight locomotor holds it at. The throttle is a fraction that rises as the aircraft sets off and falls away as it closes on its destination, and no house figure is read anywhere in that step. The difficulty multiplier that does reach movement is [`Groundspeed=`](/keys/groundspeed/#scope-difficulty-settings), and an aircraft in flight is one of the things it does not scale.
