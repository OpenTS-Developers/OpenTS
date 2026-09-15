---
key: IonAmbient
scope: random-map-generation
label: Random map generation
see_also: [UseIonStorms, "system:ion-storms"]
when_omitted:
  kind: context-dependent
  note: The ambient level generation has just set from the seed's time of day, so the storm runs at the map's ordinary light level.
---

Random map generation reads this entry from `ION.INI`. Only the name is fixed: the game looks `ION.INI` up through [the ordinary file layer](/formats/mix/), so the game-directory copy is the one generation reads. That file is where a mod sets the storm lighting every generated map will use. The read happens only when the [`UseIonStorms`](/keys/useionstorms/) option is on for the map being built.

The value is the light level a storm fades the map to, on the [Ambient](/keys/ambient/) scale where `1` is full daylight. `1` leaves the map at its generated light, below `1` darkens it for the storm's length, above `1` brightens it, and `0` is no ambient light at all. The [ambient fade](/systems/ion-storms/#the-ambient-ramp) travels to this level and back, exactly as it does for a scenario's [`IonAmbient`](/keys/ionambient/#scope-scenarios).

```ini title="ION.INI"
[Lighting]
IonAmbient=0.5  ; the stock file's figure: storms fade to half daylight
```
