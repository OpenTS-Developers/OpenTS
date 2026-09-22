---
key: NumPlayers
scope: random-map-generation
label: Players the map is built for
see_also: [Width, Height, Seed, TiberiumLayout, Tiberium]
when_omitted:
  kind: value
  value: "2"
  note: Two start points, and the first row of the size tables that Height interpolates within.
---

The figure is the number of start points the generator must find. It also chooses the row of the size tables that [`Height`](/keys/height/#scope-random-map-generation) interpolates within, so it decides how large a map of a given size index comes out. [Map seed files](/formats/map-seed/) covers the section it is written in.

```ini title="map seed file"
[RandomMap]
NumPlayers=4
Width=2
Height=2
```

Finding the start points comes late in generation, after the terrain and its regions are settled. Fifteen candidate cells per player are gathered from the largest regions reachable from one another, the most widely separated of them are taken, and the first of those become the start points. Each start point then has to flood exactly four hundred clear cells around itself. If any player's flood runs dry sooner, the start points are all discarded and picked again from the next state of the random sequence; the terrain itself is kept. Nothing caps that retry, so a player count the terrain cannot seat leaves the generator picking forever.

A seed file that leaves the figure out cannot be started at all. The skirmish and lobby screens size a game by the start points recorded in the file itself, take a missing `NumPlayers` as `0`, and turn every lineup away as too small.

The figure reaches the tiberium as well. The spread that supplied the start points supplies the map's tiberium fields from what is left over. The amount of tiberium each field is grown to is multiplied by the player count, so a map built for more players is not left thinner. [`TiberiumLayout`](/keys/tiberiumlayout/) covers how many fields there are, which the player count does not change.

When a map is [generated from a file](/systems/map-generation/#the-dialog-path-and-the-scenario-path), a value below `2` becomes `2` and one above `8` becomes `8`.
