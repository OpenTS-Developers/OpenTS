---
key: IonLevel
scope: random-map-generation
label: Random map generation
see_also: [UseIonStorms, "system:ion-storms"]
when_omitted:
  kind: context-dependent
  note: "0: generation has left the map's ordinary `Level` at a thousandth of full light by then, and the fallback truncates that to nothing. Every storm then shades the map flat."
---

Random map generation reads this entry from `ION.INI`. Only the name is fixed: the game looks `ION.INI` up through [the ordinary file layer](/formats/mix/), so the game-directory copy is the one generation reads. That file is where a mod sets the storm lighting every generated map will use. The read happens only when the [`UseIonStorms`](/keys/useionstorms/) option is on for the map being built.

The value is the brightness a storm adds for each height level a cell stands above the ground, in place of the map's ordinary [`Level`](/keys/level/#scope-scenarios). It also scales the height bonus drawn onto aircraft and onto elevated units and infantry. It is a fraction of full light per level, so cliffs and hills stand out more as the value grows, and `0` shades the map flat. The swap in and out happens on the frames the storm breaks and ends, exactly as it does for [a scenario's `IonLevel`](/keys/ionlevel/#scope-scenarios).

```ini title="ION.INI"
[Lighting]
IonLevel=0  ; the stock file's figure: storms shade the map flat
```

The fallback cannot take an ordinary height brightness across. Generation has set the map's `Level` to a thousandth of full light by then, and the fallback's whole-number division reduces that to `0`. A generated map is therefore shaded flat through every storm unless `ION.INI` states this value. A value the file states is used exactly as written; only the omission collapses to `0`.
