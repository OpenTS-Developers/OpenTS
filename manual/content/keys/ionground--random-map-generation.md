---
key: IonGround
scope: random-map-generation
label: Random map generation
see_also: [UseIonStorms, "system:ion-storms"]
when_omitted:
  kind: context-dependent
  note: "0: the fallback truncates the map's ordinary `Ground` to a whole key value, and no ordinary fraction survives it. The storm then runs with no ground darkening at all."
---

Random map generation reads this entry from `ION.INI`. Only the name is fixed: the game looks `ION.INI` up through [the ordinary file layer](/formats/mix/), so the game-directory copy is the one generation reads. That file is where a mod sets the storm lighting every generated map will use. The read happens only when the [`UseIonStorms`](/keys/useionstorms/) option is on for the map being built.

The value is the flat darkening a storm lays over the map, in place of the map's ordinary [`Ground`](/keys/ground/). It is a fraction of full light subtracted from every cell's brightness. `0` darkens nothing, and each tenth takes another tenth of full light off, so the deeper the darkening wanted, the larger the value. The swap in and out happens on the frames the storm breaks and ends, exactly as it does for [a scenario's `IonGround`](/keys/ionground/#scope-scenarios).

```ini title="ION.INI"
[Lighting]
IonGround=0  ; the stock file's figure: no flat storm darkening
```

The fallback cannot take the map's ordinary `Ground` across, so a storm erases the map's ground darkening unless `ION.INI` states this value outright. A value the file states is used exactly as written; only the omission collapses to `0`.
