---
key: Buildup
summary: The construction animation file a structure plays while it is being built.
see_also: ["BuildupTime", "DemandLoadBuildup", "GateStages", "Unsellable", "system:production"]
when_omitted:
  kind: value
  value: ""
  note: The type has no construction artwork.
---

The value is a filename without its extension. An empty value keeps the previous one, and only the first 15 characters are kept. The game loads `<value>.SHP` with the rules, or with [`DemandLoadBuildup=yes`](/keys/demandloadbuildup/) when a scenario starts or a saved game is loaded, after rewriting the name for the scenario theater by the structure-art convention that [`DoorAnim`](/keys/dooranim/) describes.

```ini title="art.ini"
[MYWEAP] ; example war factory, drawn from its own Image ID
Buildup=GAWEAPMK ; loaded as GTWEAPMK.SHP in temperate
```

The file sets both the frames and the length of the construction animation. The animation shows the first half of the file's frames in order, one per step. A [`Gate=yes`](/keys/gate/) type has [`GateStages`](/keys/gatestages/) plus one steps instead. [Buildup](/systems/production/#buildup) covers how [`BuildupTime`](/keys/builduptime/) spreads over those steps and what happens while the animation runs.

## A type with no construction artwork cannot be sold

A structure whose type has no construction artwork can never be sold. That applies whether `Buildup` is missing or names a file the game cannot find. The sell cursor, the [Sell building](/mapping/actions/taction-sell-attached/) trigger action and the computer's sell-offs all refuse it. The one exception is a [`FirestormWall=yes`](/keys/firestormwall/) type, which a sell order removes at once. For the same reason, such a structure cannot be ordered to undeploy into its [`UndeploysInto`](/keys/undeploysinto/) vehicle.

[`Unsellable=yes`](/keys/unsellable/) is the deliberate way to prevent a sale, and it is weaker. It blocks the sell cursor, but the trigger action and the computer's sell-offs still sell the structure.

Each structure checks for construction artwork once, as it is created. A [`Nominal=yes`](/keys/nominal/) survivor running from a destroyed structure becomes a technician only when that check found construction artwork.

:::caution[A theater-specific structure is timed differently]
A [`Theater=yes`](/keys/theater/) structure has its construction artwork fetched again as the theater is set up. This fetch uses the theater's own extension in place of `.SHP` and does not rewrite the name. It counts every frame in the file as a step and spreads five seconds over them, so neither `BuildupTime` nor the halving above applies. If no file with that extension exists, the type has no construction artwork, unless the map redefines the type afterward.
:::
