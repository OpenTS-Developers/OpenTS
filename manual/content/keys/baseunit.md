---
key: BaseUnit
summary: The UnitTypes handed out at a multiplayer start, one per house, and counted as a base of their own.
see_also: ["system:crates", "system:starting-forces", DeploysInto]
when_omitted:
  kind: value
  value: none
---

Nothing about a type itself marks it out; every effect follows from being named here. The one property of the type that is read is its [`DeploysInto`](/keys/deploysinto/). Where one is handed out, the house takes the first entry the country it [acts as](/keys/actslike/) may own, or entry 0 when it may own none, so naming one MCV per country gives each faction its own; a single value is a list of one and behaves as it always did.

- With bases enabled, each house that is not passive is given its entry on its start position, or on the nearest cell the [placement search](/systems/starting-forces/#where-an-object-lands) finds within thirty-one cells, and it is the object a house's flag is attached to in capture-the-flag. Enabling bases also takes one off the lobby's unit count to pay for it, when the list names anything.
- The random starting units are drawn from the [`AllowedToStartInMultiplayer=yes`](/keys/allowedtostartinmultiplayer/) types with every listed type held out, and the [average price](/systems/starting-forces/#the-budget) that decides how many are drawn leaves it out too.
- In a short game, a house holding no structures and no unit of any listed type is defeated.
- A crate collected by a house that has lost its base and can afford to rebuild [delivers its entry](/systems/crates/#money-and-free-units), and with bases disabled the random vehicle result refuses to hand out any listed type.
- The [Center Base](/commands/centerbase/) command looks for a [`BuildConst`](/keys/buildconst/) structure, and falls back to a unit of any listed type when it finds no building of the player's at all.

An empty list is not an error: no base unit is placed, the unit count is left whole, and a short game is lost with the last structure.
