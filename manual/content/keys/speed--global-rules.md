---
key: Speed
scope: global-rules
label: Jumpjet travel ceiling
see_also: [Acceleration, TurnRate]
when_omitted:
  kind: value
  value: "30"
---

The jumpjet locomotor keeps a speed counter of its own and slides the unit that many leptons along its facing every game frame. This figure is the ceiling that counter is clamped to, so it is the top travel rate every jumpjet in the game shares. At the engine default of `30` that is a little under two cells a second. [`Acceleration`](/keys/acceleration/#scope-global-rules) decides how quickly the counter reaches it.

```ini title="rules.ini"
[JumpjetControls]
Speed=20  ; about 1.2 cells a second
```

A unit's own [`Speed=`](/keys/speed/#scope-aircrafttype) does not move a jumpjet. What the locomotor hands the unit instead is a throttle: the counter divided by this figure. The type's speed rating is left to the places that read a moving object's rate second hand, such as another object leading its shot at one.

The slower bands the locomotor drops to as it arrives are fractions of this figure. It runs at three tenths within one cell of the destination and one half within two, with the counter zeroed outright inside the last 20 leptons. Those proportions hold whatever the ceiling is set to.
