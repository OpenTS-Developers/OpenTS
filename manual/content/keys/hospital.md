---
key: Hospital
summary: Admits one damaged infantry at a time and heals it to full strength.
see_also: ["system:repair"]
when_omitted:
  kind: value
  value: "no"
---

A building with this flag offers the enter cursor to a player-controlled infantry of an allied house that is below full strength, under **All of:**

- the building is out of construction;
- it is switched on;
- it is not already servicing someone;
- it still holds ammunition.

Admission costs one point of the building's [`Ammo`](/keys/ammo/) pool. A hospital never receives the instant restock that refills an emptied ammunition pool, so the pool only ever goes down.

Healing then runs on [`IRepairRate`](/keys/irepairrate/): each time the counter elapses the occupant gains [`IRepairStep`](/keys/irepairstep/) strength at no charge, and it is released once it reaches full strength. An occupant that turns out to need nothing is released at the first count, having still spent the admission point.

:::caution[An unset ammunition pool allows exactly one visit]
A building type that names no `Ammo` starts its pool at `-1`, which passes the non-zero entry test; the decrement on the first admission then clamps the pool to zero, and the enter cursor is refused from then on. Give the type an explicit `Ammo` count for the number of patients it should treat.
:::
