---
key: CarriesCrate
summary: Whether a destroyed vehicle of this type drops a crate, when the scenario allows the drop.
see_also: ["system:crates"]
when_omitted:
  kind: value
  value: "no"
---

The scenario decides whether the drop happens at all. A destroyed vehicle with the flag drops a crate under **Any of:**

- **All of:** its type is not [`IsTrain=yes`](/keys/istrain/), and the scenario sets [`TruckCrate=yes`](/keys/truckcrate/);
- **All of:** its type is `IsTrain=yes`, and the scenario sets [`TrainCrate=yes`](/keys/traincrate/).

With neither scenario flag set, the type's own flag has no visible consequence.

The crate is placed on a nearby cell that has no overlay and is passable to tracked movement, and it uses the same overlay as a randomly placed crate. It is not registered with the crate tracker, so unlike a random crate it never expires.
