---
key: FillSilos
summary: Tops every house's Tiberium storage up as the scenario opens.
see_also: ["system:tiberium", Storage]
when_omitted:
  kind: value
  value: "no"
---

```ini title="map file"
[Basic]
FillSilos=yes
```

Once the scenario has finished loading, every house on the map is handed Tiberium of the first registered type, one unit at a time. The handing stops when the house's free storage runs out, or once the units handed over are worth at least the starting money minus one unit's worth. The Tiberium is spread across the house's storage-capable buildings exactly as a harvester unload would spread it, so silos and refineries stand stocked from the first frame.

The setting is read for every game type, not campaigns alone. In a multiplayer or skirmish game a computer house takes a different route through the same delivery and is credited the money outright instead of being given anything to store.

:::caution[The money is not moved, it is matched]
The starting credits only set the amount to measure against; nothing deducts them. A house with storage to spare therefore begins richer than it would have, holding both its credits and Tiberium worth up to the same figure again. A house with no storage-capable building gains nothing at all.
:::
