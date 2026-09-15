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

Having a weapon changes where the vehicle goes when it has nothing to do, not the harvest work itself. An unarmed weeder starts on the harvest mission and returns to it whenever it idles. A player-owned one that idles away from vein ground takes plain guard until something orders it back. An armed one idles into the guard behavior of an ordinary combat vehicle, and runs the harvest cycle only while the harvest mission is assigned to it.

```ini title="rules.ini"
[MYWEEDEATER] ; a UnitType registered in [VehicleTypes]
Weeder=yes
```
