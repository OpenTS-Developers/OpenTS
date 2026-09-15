---
key: SpawnCount
scope: animtype
label: Animation spawn count
see_also: ["Spawns", "Bouncer", "IsMeteor"]
when_omitted:
  kind: value
  value: "0"
---

The number of children is the sum of two independent picks from zero up to this figure, so it runs from none to twice the setting and averages the setting itself. A figure of zero or below spawns nothing, whatever [`Spawns`](/keys/spawns/#scope-animtype) names.

Only a thrown animation spawns at all: one with [`Bouncer=yes`](/keys/bouncer/) or [`IsMeteor=yes`](/keys/ismeteor/#scope-animtype), and not one that ends low over water.
