---
key: ExtraUnitLight
summary: Extra brightness every vehicle is drawn with, as a fraction of full light.
see_also: [ExtraInfantryLight, ExtraAircraftLight]
when_omitted:
  kind: value
  value: "0"
  note: "Any file that declares `[AudioVisual]` re-reads the key and hands the read the stored value divided by full light as whole numbers, so a setting below full light comes back as `0`."
---

```ini title="rules.ini"
[AudioVisual]
ExtraUnitLight=.2
```

The figure is written as a fraction of full light, where `1` is the full strength the map's own lighting is measured against. It is added to the brightness the cell and the current ambient level already give the vehicle, after the lift a bridge deck gives it and after the darkening an overshadowed cell applies. `.2` therefore lifts a vehicle a fifth of full strength above its surroundings wherever it stands.

Three draws take it: the vehicle body, the separate harvesting overlay drawn beside a working harvester, and the flag a vehicle carries in a capture-the-flag game. Nothing outside the drawing of a vehicle reads the setting, so it changes no targeting, cloaking or lighting calculation.

:::caution[A later rules file resets this key]
The figure is kept as a whole number of light units, full light being 1000, so the `.2` above is stored as 200. Every later read of the key is handed the stored figure divided by full light as its default. That division is whole-number: 200 becomes `0`, and `0` multiplied back by full light is stored as `0`. Any later file that declares `[AudioVisual]` therefore wipes the setting unless it repeats this assignment. `langrule.ini`, the Firestorm rules file, and a scenario map all count, however few keys they set.
:::
