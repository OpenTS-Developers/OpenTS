---
key: Inert
summary: Strips all damage out of the scenario's weapons.
when_omitted:
  kind: value
  value: "no"
  note: The special options are initialized with this built-in default when the game starts.
---

Two routines run the whole effect. The one that converts a weapon's raw damage into damage against a particular armor returns zero, and the one that spreads an explosion over the objects around a point returns before it collects anything. Nothing on the map can then be destroyed by weapon fire. Everything else about firing carries on: weapons still reload, projectiles still travel, and the report and impact effects still play.

Damage delivered as forced skips the warhead conversion altogether and is untouched, so an infantryman standing in Tiberium still dies of it.

:::caution[The entry is read in campaigns only]
The `[SpecialFlags]` block is read from the map only in a single-player mission. Every other game type replaces the scenario's whole set of flags with the set the game options carry once the map has been read.
:::
