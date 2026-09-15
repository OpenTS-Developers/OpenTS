---
key: WallOwner
summary: Whether the country's buildings can claim ownership of nearby walls placed with the map.
see_also: ["system:walls-and-gates"]
when_omitted:
  kind: value
  value: "yes"
---

Walls that arrive with the map have no owner. After the map is loaded, the final map pass hands each wall cell to the owner of the nearest building that is active, placed, and owned by a country with this flag. The search covers every building in the game, not a radius around the wall, and a tie at equal distance goes to whichever the game created first. A map with no such building anywhere leaves its walls unowned. An unowned wall cannot be sold and is not an anchor for the [automatic gap fill](/systems/walls-and-gates/#filling-the-gap-to-the-next-wall). Walls the player builds are stamped with their house directly and never read this.

:::caution[The value is overwritten outside campaign]
Skirmish, multiplayer and random-map setup rewrite the flag before play begins, on every country that has a house in the game: a [`MultiplayPassive=yes`](/keys/multiplaypassive/) country is forced to `no` and every other country to `yes`. An authored `WallOwner=` therefore only holds in a campaign game.
:::
