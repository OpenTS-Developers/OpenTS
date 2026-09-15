---
key: Airspeed
scope: housetype
label: Country air-speed multiplier
see_also: ["system:difficulty", Groundspeed]
when_omitted:
  kind: value
  value: "1.0"
---

Each country sets its own multiplier in its own section. A house of that country combines it with [the difficulty section's](/keys/airspeed/#scope-difficulty-settings) and with [`GameSpeedBias`](/keys/gamespeedbias/) once, [when it is given its difficulty slot](/systems/difficulty/#how-the-figures-are-combined). A campaign game drops the country's contribution and keeps the other two, which is how [`Groundspeed=`](/keys/groundspeed/#scope-housetype) is treated as well.

No gameplay path reads the combined figure, so neither this multiplier nor the difficulty setting's changes an aircraft's speed. An aircraft's flying speed comes from [`Speed=`](/keys/speed/#scope-aircrafttype) in the aircraft type's own section instead.
