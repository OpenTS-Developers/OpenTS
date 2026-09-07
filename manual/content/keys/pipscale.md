---
key: PipScale
summary: The quantity the pip row under a selected object counts, and how many pips that row has.
see_also: [Passengers, Size, Ammo, Storage, MaxCharge, Pip, "system:transports"]
when_omitted:
  kind: value
  value: none
---

```ini title="rules.ini"
[MYAPC] ; a UnitType registered in [VehicleTypes]
Passengers=5
PipScale=Passengers
```

The setting does two jobs at once: it chooses which quantity the row reports, and it fixes how long the row is. Left unset, the row has no pips at all, so a transport or a harvester with no `PipScale` shows nothing however full it is. Omitting the key keeps whatever an earlier rules layer set, but a value that matches none of the [pip scales](/reference/enums/pip-scale/) does not: it discards the stored value and leaves the row empty.

| Value | Row length | What fills it |
| --- | --- | --- |
| `Ammo` | [`Ammo`](/keys/ammo/), capped at 5 | rounds remaining, drawn as filled markers with no empty ones behind them |
| `Tiberium` | 5 on a vehicle | stored Tiberium against [`Storage`](/keys/storage/) |
| `Passengers` | [`Passengers`](/keys/passengers/), capped at 5 | the hold |
| `Power` | 10 | nothing — no drawing branch answers to this value |
| `Charge` | 8 | a vehicle's charge against [`MaxCharge`](/keys/maxcharge/) |

A structure sizes its row differently. Its width allowance is six pips per cell of footprint width; `Tiberium` caps that by its own [`Storage`](/keys/storage/) — or by [`[General] WeedCapacity`](/keys/weedcapacity/) on a [`Weeder=yes`](/keys/weeder/#scope-buildingtype) structure — and `Power` grants the whole allowance. A structure set to `Tiberium` with no storage of its own is given a row of no length and shows nothing; only a [`Weeder=yes`](/keys/weeder/) structure reaches the house-wide reading, and a weeder reports its house's weed fraction.

The row's *contents* change once the type carries [`Passengers`](/keys/passengers/) above zero. Such an object draws one pip per slot, colored by an infantry passenger's own [`Pip`](/keys/pip/), and green for anything else, instead of the scaled fraction above. The scale is still what decides how many slots are drawn, so a transport set to `PipScale=Ammo` shows its cargo in a row sized by its ammunition.

A vehicle set to `Tiberium` splits its row by what it is carrying: the first Tiberium type draws green pips and everything else draws blue.
