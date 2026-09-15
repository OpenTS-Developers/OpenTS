---
key: TiberiumDeathToVisceroid
summary: Whether infantry killed by Tiberium leave a visceroid behind.
see_also: [SmallVisceroid, Visceroids, "system:tiberium"]
when_omitted:
  kind: value
  value: "yes"
---

```ini title="map file"
[Basic]
TiberiumDeathToVisceroid=no
```

Two deaths are covered: an infantryman standing in a Tiberium cell who succumbs to the poisoning, and one killed by a Tiberium gas particle. Each one creates a [`SmallVisceroid`](/keys/smallvisceroid/) belonging to the `Neutral` house on the spot, provided no vehicle already occupies that cell.

Nothing else spawns visceroids from infantry deaths, so clearing the switch removes them from the scenario entirely; the `[SpecialFlags]` entry named [`Visceroids`](/keys/visceroids/) is not read at all.
