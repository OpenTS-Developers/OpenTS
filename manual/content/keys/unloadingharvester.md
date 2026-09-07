---
key: UnloadingHarvester
summary: The vehicle type a harvester is drawn as while it stands at a refinery unloading.
see_also: [UnloadingClass, Harvester, Weeder, Dock, "system:tiberium"]
when_omitted:
  kind: value
  value: none
---

```ini title="rules.ini"
[AudioVisual]
UnloadingHarvester=HORV ; a UnitType registered in [VehicleTypes]
```

While a [`Harvester=yes`](/keys/harvester/#scope-unittype) vehicle is docked and handing its load over, its own type is set aside and this one is put in its place for the length of the draw, then restored. [Unloading](/systems/tiberium/#unloading) covers the docking that gets it there. A [`Weeder=yes`](/keys/weeder/#scope-unittype) vein harvester that is not also a Tiberium harvester unloads through the same states but is not reached from here. A vehicle that names its own [`UnloadingClass`](/keys/unloadingclass/) uses that instead of this rules-wide value, and that key is the only one a vein harvester reads.

Leaving both unset draws the harvester with its own artwork throughout, which is what a mod whose harvester carries its own unloading animation wants.

:::caution[The whole type is exchanged, not only the image]
Everything the draw takes from the vehicle's type comes from the substitute for that frame — its artwork, whether it is a voxel or a shape, and the drawing settings that go with it. A substitute that is not built to stand in for the harvester can therefore change its size, its facing behavior and its shadow, not merely its appearance.
:::
