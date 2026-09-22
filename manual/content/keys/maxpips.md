---
key: MaxPips
summary: How many pips a selected object's pip row holds.
see_also: [PipScale, Passengers, Ammo, Storage, MaxCharge, "system:transports"]
when_omitted:
  kind: context-dependent
  note: "The length the object's pip scale gives. That is five for `Tiberium`, ten for `Power` and eight for `Charge`, and for `Ammo` and `Passengers` it is the type's `Ammo` or `Passengers` value, at most five. A structure using `Tiberium` or `Power` gets six pips per cell of footprint width instead."
---

```ini title="rules.ini"
[MYAPC]           ; a UnitType registered in [VehicleTypes]
Passengers=12
PipScale=Passengers
MaxPips=12
```

`MaxPips` sets how many pips the row has, replacing the default length of the type's [`PipScale`](/keys/pipscale/). The example transport shows twelve passenger pips instead of five. A type with no `PipScale` has no row, so the key does nothing for it.

Under `PipScale=Ammo` and `PipScale=Passengers`, the row is still capped by the type's [`Ammo`](/keys/ammo/) or [`Passengers`](/keys/passengers/) value. `MaxPips=20` on a transport with `Passengers=6` draws six pips.

On a structure using [`PipScale=Tiberium`](/keys/pipscale/) or `PipScale=Power`, the key replaces the default of six pips per cell of footprint width. Under `Tiberium` the row is still capped by the structure's [`Storage`](/keys/storage/), or by `[General] WeedCapacity` on a weeder.

A negative value counts as `0`, which leaves the row with no pips.

:::caution[A long row runs past its object]
A vehicle, infantry or aircraft draws its row to the right, four pixels per pip, and nothing limits the row to the object's width. A row much longer than the object is wide is drawn over whatever stands beside it. A structure's default length is sized to its footprint, and a value set here can make the row run past the structure's edge too.
:::
