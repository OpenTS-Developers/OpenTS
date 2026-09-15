---
key: UnitCrateType
summary: The single vehicle type every unit crate delivers.
see_also: ["system:crates"]
when_omitted:
  kind: value
  value: none
---

With a type named here, the unit result always creates that type. With `UnitCrateType=none` the result falls back to the two rescues described below. When neither applies, it draws repeatedly from the registered UnitTypes until it finds one that is [`CrateGoodie=yes`](/keys/crategoodie/) and ownable by the collector's house.

:::caution[Naming a type cancels both free-vehicle rescues]
The unit result first checks whether the collector's house has lost its base and can afford to rebuild, and then whether it owns a refinery but no unit of the first [`HarvesterUnit`](/keys/harvesterunit/) type. It chooses the MCV or a harvester accordingly. This setting is applied after both and overwrites either one, so naming a type here means a house that has lost its base receives that type instead of an MCV.
:::
