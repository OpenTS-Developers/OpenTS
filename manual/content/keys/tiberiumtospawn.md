---
key: TiberiumToSpawn
summary: Registration slot of the Tiberium type a Tiberium-spawning TerrainType produces.
see_also: ["system:tiberium", "SpawnsTiberium"]
when_omitted:
  kind: value
  value: "0"
---

The value is a slot number in the order the `[Tiberiums]` list registers its types, not a section name, so `0` means the first registered type. It is read only while the object declares [`SpawnsTiberium=yes`](/keys/spawnstiberium/), and it decides both the type the object's cell reports and the type its animation seeds.
