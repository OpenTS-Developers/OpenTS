---
key: IonGreen
scope: random-map-generation
label: Random map generation
see_also: [UseIonStorms, "system:ion-storms"]
when_omitted:
  kind: context-dependent
  note: The green tint the scenario is holding when generation reads this file.
---

Random map generation reads this entry from `ION.INI`. Only the name is fixed: the game looks `ION.INI` up through [the ordinary file layer](/formats/mix/), so the game-directory copy is the one generation reads. That file is where a mod sets the storm lighting every generated map will use. The read happens only when the [`UseIonStorms`](/keys/useionstorms/) option is on for the map being built.

The value is the storm's green tint, a multiplier on the green channel of every terrain palette color. It uses the same scale as the [ordinary tint keys](/keys/green/): `1` leaves the channel alone, above `1` adds green, below `1` takes it away, and `0` removes the channel outright. The engine clamps the value to the range 0 through 2. The three tints replace the map's ordinary colors on the frame the storm breaks and go back when it ends, exactly as [a scenario's `IonGreen`](/keys/iongreen/#scope-scenarios) works.

```ini title="ION.INI"
[Lighting]
IonGreen=1.25  ; the stock file's figure: the storm's green push
```
