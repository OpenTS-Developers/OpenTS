---
key: Armory
summary: Admits one infantry at a time and promotes it when the servicing delay elapses.
see_also: ["system:veterancy"]
when_omitted:
  kind: value
  value: "no"
---

A building with this flag offers the enter cursor to a player-controlled infantry of an allied house, provided that infantry is not already elite. The building itself must meet **all of** the following:

- it is switched on;
- it is out of construction;
- it is not already servicing someone;
- it still holds ammunition.

Admitting the infantry costs one point of the building's [`Ammo`](/keys/ammo/) pool, and an armory, like a hospital, is exempt from the instant ammunition reload other buildings receive, so the pool only ever goes down.

Once the servicing counter set by [`IRepairRate`](/keys/irepairrate/) runs out, the occupant is promoted and released. [Promotion without kills](/systems/veterancy/#promotion-without-kills) covers where that promotion sits among the other non-combat sources.

:::caution[The promotion skips the veteran rank]
An occupant below rookie is raised to veteran; everything else is raised straight to elite. A rookie therefore leaves the building elite, while a veteran gains a rank it could have reached by fighting.
:::

:::caution[An unset ammunition pool allows exactly one visit]
A building type that names no `Ammo` starts its pool at `-1`, which passes the non-zero entry test; the decrement on the first admission then clamps the pool to zero. Because an armory is never restocked, the enter cursor is refused from then on, so the building promotes exactly one infantry in its lifetime. Give the type an explicit `Ammo` count for the number of promotions it should hand out.
:::
