---
key: Speed
scope: aircrafttype
label: Object top speed
see_also: ["Accelerates", "SpeedType"]
when_omitted:
  kind: value
  value: "0"
  note: Zero is immobile. Every stock type that moves sets the key.
---

The figure is a percentage, not a distance. It is clamped to the range 0 to 100 and then scaled onto the engine's internal 0-to-255 speed scale. `Speed=100` is the fastest any type can be given, `Speed=50` stores 128, and a figure above 100 is no faster than 100.

The scaled figure is the ground the object covers in one game frame, before anything scales it. The scalers are [the house's combined ground speed bias](/systems/difficulty/#how-the-figures-are-combined), the veteran speed ability, a limpet drone clamped to a vehicle, the throttle its locomotor is holding it at, and any speed bias a crate has given it. A vehicle with the flag in capture-the-flag travels at half the result.

```ini title="rules.ini"
[HARV]
Speed=5 ; stores 12 of 255
```

:::caution[Writing `-1` is the same as leaving the key out]
The read uses `-1` as its own marker for a missing key, so `Speed=-1` leaves whatever speed was already stored rather than storing anything. A rules file that means to make a type immobile has to write `Speed=0`.
:::
