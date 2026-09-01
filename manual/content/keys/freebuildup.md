---
key: FreeBuildup
summary: Releases the structure's construction artwork once it is no longer needed.
see_also: ["Buildup", "DemandLoadBuildup"]
when_omitted:
  kind: value
  value: "no"
---

With [`DemandLoadBuildup=yes`](/keys/demandloadbuildup/), this flag releases construction art after draw-area measurement, structure creation and its sellability check, buildup completion, and structure destruction. The next request reloads it.

Without `DemandLoadBuildup=yes`, it does nothing and leaves archive art attached. Prior OpenTS stripped that art after the first structure, so later structures lost construction and deconstruction, sellability, and technician conversion for nominal crew on destruction.
