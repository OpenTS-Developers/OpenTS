---
key: TeslaZap
summary: Parsed sound that no turret ever discharges.
no_effect: true
see_also: [TeslaCharge, Charges, IsLaser]
when_omitted:
  kind: value
  value: none
---

The name promises the discharge of a charging turret. The value is resolved against the sound registry and stored, and no gameplay path reads the slot back.

Its companion is live: a structure whose primary weapon sets [`Charges=yes`](/keys/charges/), and which is switched on, fully powered and has a target, plays [`TeslaCharge`](/keys/teslacharge/) at its own position as the wind-up starts. The shot that follows makes only the sounds its own weapon names.
