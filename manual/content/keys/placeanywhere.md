---
key: PlaceAnywhere
summary: Passes every placement legality test without examining a single cell.
see_also: [Buildable, DeploysInto, "system:base-adjacency"]
when_omitted:
  kind: value
  value: "no"
---

The legality test a BuildingType is put through ordinarily walks its whole foundation and asks each covered cell whether it is clear to build. The cell qualifies under **All of:**

- nothing already occupying it;
- inside the playable area;
- no overlay in the way;
- off a bridge;
- off a cell under a bridge;
- off a ramp;
- on a land type with [`Buildable=yes`](/keys/buildable/).

A type that paves what it covers needs only one cell of the foundation to pass; every other type needs all of them.

This flag answers that test with success before any of it runs. The whole foundation is skipped, and so is the guard that would refuse a placement with no cell at all behind it. Six paths ask the question and take the answer it gives:

- The structure's own arrival on the map. A structure that fails the test is never put down; a flagged one is put down wherever it was asked for.
- The deploy cursor on a vehicle whose [`DeploysInto`](/keys/deploysinto/) names the type.
- The deploy order on that vehicle.
- The search a vehicle runs for a nearby spot it could deploy on.
- The unload step of a team script, which otherwise clears the cell and waits.
- The computer's search for a spot in its base, which still applies its own spacing, height and adjacency conditions on top.

:::caution[The placement cursor does not read the flag]
The cursor drawn while a player holds a pending structure asks each covered cell whether it is clear to build directly, rather than through the type's own legality test. A flagged structure therefore keeps showing the blocked cursor over ground it could not otherwise take. The click is gated on [adjacency](/systems/base-adjacency/) and shroud alone, so the placement then succeeds anyway.
:::
