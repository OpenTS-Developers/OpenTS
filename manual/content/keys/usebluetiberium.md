---
key: UseBlueTiberium
summary: Grows some of a generated map's fields from the second tiberium overlay set rather than the first.
see_also: [Tiberium, TiberiumLayout, "system:tiberium"]
when_omitted:
  kind: value
  value: "no"
  note: The fallback of `no` grows every field on the map from the first tiberium overlay set.
---

Clear, every field on the map is grown from the first overlay set. Set, each of the map's spread fields is rolled for separately, and about three in ten are grown from the second set instead. The fields planted at the players' start points are settled by one roll taken once for all of them, which gives the second set about one time in four. Those fields therefore always match one another, whichever set they take. [Map seed files](/formats/map-seed/) covers the section it is written in.

```ini title="map seed file"
[RandomMap]
UseBlueTiberium=yes
Tiberium=60
```

The setting also reaches the tiberium trees. A spread field grown from the first set has a one-in-four chance of a tree at its heart, and a field grown from the second set never has one. A start point's field has a tree whenever it took the first set. So turning the setting on also thins the trees out.

The generator itself asks no questions about the Firestorm addon here, so a seed file may set the flag whether or not that addon is present. The map generator dialog is where the addon is checked. It does not offer the flag as a control at all: it clears the flag and then, under Firestorm, sets it from [`Tiberium`](/keys/tiberium/#scope-random-map-generation).

:::caution[The dialog turns the setting on for every map it builds outside a tournament]
The comparison the dialog derives the flag from tests the tiberium richness against three quarters, as though that figure were a fraction rather than the whole number from `1` to `100` it is held to. Every richness the dialog offers outside a tournament game is above three quarters, so under the Firestorm addon a map built or previewed from the dialog always has the flag set. The randomize button's own coin toss is overwritten before the map is generated.
:::
