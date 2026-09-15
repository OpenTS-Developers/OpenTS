---
key: SpawnsTiberium
summary: Makes a TerrainType seed Tiberium into the cells around it.
see_also: ["system:tiberium", "TiberiumToSpawn"]
when_omitted:
  kind: value
  value: "no"
---

The object's own cell reports the Tiberium type named by [`TiberiumToSpawn`](/keys/tiberiumtospawn/) even with no overlay present, so infantry standing there are poisoned. The cell is also refused as a target for new growth, which is what keeps a blossom tree sitting on bare ground. A blossom-tree cell is worth nothing to a harvester, because worth comes from the overlay.

An object that also declares [`IsAnimated=yes`](/keys/isanimated/) seeds one neighboring cell at growth stage 5 each time its animation reaches the middle frame. The animation then halts there until chance restarts it. That seeding skips the test the source cell would otherwise have to pass, so it happens however bare the ground beneath the object is.
