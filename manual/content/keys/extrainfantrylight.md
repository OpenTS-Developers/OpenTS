---
key: ExtraInfantryLight
summary: Extra brightness every infantryman is drawn with, as a fraction of full light.
see_also: [ExtraUnitLight, ExtraAircraftLight]
when_omitted:
  kind: value
  value: "0"
  note: "Any file that declares `[AudioVisual]` re-reads the key and hands the read the stored value divided by full light as whole numbers, so a setting below full light comes back as `0`."
---

```ini title="rules.ini"
[AudioVisual]
ExtraInfantryLight=.2
```

The figure is written as a fraction of full light, where `1` is the full strength the map's own lighting is measured against. It is added to the brightness the cell and the current ambient level already give the soldier, after the lift a bridge deck gives him and after the darkening an overshadowed cell applies.

Only the soldier's own body takes it. The flat silhouette drawn on the ground beneath an infantryman standing above the terrain is drawn at full light whatever this setting says, so raising this setting separates a soldier from his shadow rather than lighting both.

:::caution[A later rules file resets this key]
The figure is kept as a whole number of light units, full light being 1000, so the `.2` above is stored as 200. Every later read of the key is handed the stored figure divided by full light as its default. That division is whole-number: 200 becomes `0`, and `0` multiplied back by full light is stored as `0`. Any later file that declares `[AudioVisual]` therefore wipes the setting unless it repeats this assignment. `langrule.ini`, the Firestorm rules file, and a scenario map all count, however few keys they set.
:::
