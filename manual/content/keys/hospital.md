---
key: Hospital
summary: Admits one damaged infantry at a time and heals it to full strength.
see_also: ["system:repair"]
when_omitted:
  kind: value
  value: "no"
---

A hospital heals damaged infantry free of charge, one at a time. With the player's infantry selected, pointing at an allied hospital shows the enter cursor when the infantry is below full strength and **all of** the following hold:

- the hospital has finished construction and is not being sold;
- it is switched on;
- it is not already treating another infantry;
- its [`Ammo`](/keys/ammo/) count is not zero.

Otherwise the cursor shows that the infantry cannot enter.

Each admission uses one point of `Ammo`. Most structures refill an empty `Ammo` count at once. A hospital never does, and neither does an [`Armory=yes`](/keys/armory/) structure, so every point is one patient.

The patient gains [`IRepairStep`](/keys/irepairstep/) strength each time the [`IRepairRate`](/keys/irepairrate/) interval passes, and leaves once it reaches full strength. A patient that is already at full strength when the first interval passes leaves at once, and its admission point is still spent.

A hospital can hold a [rally point](/systems/production/#rally-points). A released patient walks to it once it exits the hospital.

:::caution[Set `Ammo` to the number of patients]
A type that sets no `Ammo` starts with a count of `-1`. That count passes the admission test once, and the first admission drops it to `0`, so the hospital treats one patient and then refuses everyone. Give the type an explicit `Ammo` count.
:::
