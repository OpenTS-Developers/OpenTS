---
key: PipScale
summary: The quantity the pip row under a selected object counts, and how many pips that row has.
see_also: [Passengers, Size, Ammo, Storage, MaxCharge, MaxPips, Pip, "system:transports"]
when_omitted:
  kind: value
  value: none
---

```ini title="rules.ini"
[MYAPC] ; a UnitType registered in [VehicleTypes]
Passengers=5
PipScale=Passengers
```

`PipScale` chooses which quantity the row shows and sets the row's default length. Without a `PipScale` the row has no pips, so a transport or harvester shows nothing however full it is. Omitting the key keeps the value an earlier rules layer set, but a value that matches none of the [pip scales](/reference/enums/pip-scale/) replaces it with none and leaves the row empty.

| Value | Row length | What fills it |
| --- | --- | --- |
| `Ammo` | [`Ammo`](/keys/ammo/), capped at 5 | rounds remaining, drawn as filled markers with no empty ones behind them |
| `Tiberium` | 5 on a vehicle | stored Tiberium against [`Storage`](/keys/storage/) |
| `Passengers` | [`Passengers`](/keys/passengers/), capped at 5 | the hold |
| `Power` | 10 | nothing; no drawing branch answers to this value |
| `Charge` | 8 | a vehicle's charge against [`MaxCharge`](/keys/maxcharge/) |

[`MaxPips`](/keys/maxpips/) replaces these lengths. The `Ammo` and `Passengers` rows stay capped by the type's [`Ammo`](/keys/ammo/) and [`Passengers`](/keys/passengers/) values.

On a structure, the `Tiberium` and `Power` rows default to six pips per cell of footprint width instead of the table's length, or [`MaxPips`](/keys/maxpips/) where the type sets it. Under `Tiberium` that length is capped by the structure's own [`Storage`](/keys/storage/), so a structure with no `Storage` shows no row. A [`Weeder=yes`](/keys/weeder/#scope-buildingtype) structure is capped by [`[General] WeedCapacity`](/keys/weedcapacity/) instead, and its row shows how much weed its whole house holds.

A type with [`Passengers`](/keys/passengers/) above zero shows its hold instead of the quantity in the table: one pip per unit of passenger space, colored by an infantry passenger's [`Pip`](/keys/pip/), green for any other passenger, and empty for free space. The scale still sets the row's length, so passengers past the end of the row get no pip, and a transport with `PipScale=Ammo` shows its cargo in a row sized by its ammunition.

A vehicle set to `Tiberium` splits its row by what it is carrying: the first Tiberium type draws green pips and everything else draws blue.
