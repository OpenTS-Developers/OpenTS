---
key: Weeder
scope: unittype
label: Vein harvester
see_also: ["system:veins", "system:tiberium", "Harvester", "Storage", "Dock"]
when_omitted:
  kind: value
  value: "no"
---

The flag puts the vehicle on the vein branch of the harvest mission. It fills from cells of the [Weeds](/reference/enums/land-type/) land type that hold mature vein, up to its [`Storage`](/keys/storage/) capacity. Delivery goes to a [`Weeder=yes`](/keys/weeder/#scope-buildingtype) building of its own house, drawn from its [`Dock`](/keys/dock/) list. [Weed harvesting](/systems/veins/#weed-harvesting) owns the patch search and the load and unload cycles, [Tiberium harvesting](/systems/tiberium/#harvesting) owns the mission the two branches share, and each delivery is credited to the house's [weed pool](/systems/veins/#the-weed-pool).

A weeder starts harvesting when it is placed on the map, including when it leaves a factory or a repair bay. When it later goes idle, it resumes harvesting if its house is computer-controlled or it is standing on veins. A player-owned weeder that goes idle anywhere else takes guard until it is given another order; an armed one may take area guard instead, as other armed vehicles do.

```ini title="rules.ini"
[MYWEEDEATER] ; a UnitType registered in [VehicleTypes]
Weeder=yes
```
