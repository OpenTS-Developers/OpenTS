---
enum_id: PipScaleType
slug: pip-scale
title: Pip scale
summary: Quantities that a selected object's pip row can represent.
representation: token
bindings:
  key_value_types: [PipScaleType]
  scripting_parameter_types: []
source_files: [code/pip.hh, code/ccini.cpp]
values:
  - { constant: PIPSCALE_AMMO, value: 1, input: "Ammo", meaning: "Remaining ammunition." }
  - { constant: PIPSCALE_TIBERIUM, value: 2, input: "Tiberium", meaning: "Stored Tiberium or resource cargo." }
  - { constant: PIPSCALE_PASSENGERS, value: 3, input: "Passengers", meaning: "Occupied passenger slots." }
  - { constant: PIPSCALE_POWER, value: 4, input: "Power", meaning: "No quantity; the row is given a length and no drawing branch fills it." }
  - { constant: PIPSCALE_CHARGE, value: 5, input: "Charge", meaning: "A vehicle's stored charge against its MaxCharge." }
---

A **pip** is one of the small markers drawn in a row beneath a selected object, and the scale names the quantity that row is counting. The [pip colors](/reference/enums/pip-color/) are a separate domain and decide what an individual marker looks like.

The five scales are fixed by the engine, and the match ignores case. [`PipScale`](/keys/pipscale/) covers how each one sizes and fills its row. A `Charge` row is measured against [`MaxCharge`](/keys/maxcharge/), and a `Power` row is given a length that nothing fills.
