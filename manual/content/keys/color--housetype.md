---
key: Color
scope: housetype
label: Country color
see_also: [Multiplay, Side]
when_omitted:
  kind: computed
  note: The first color scheme in the loaded list.
---

The value is a color scheme name, matched without regard to letter case against [the schemes the rules declare in `[Colors]`](/keys/color/). A scheme with only one intensity level is skipped during that match, and a name matching no remaining scheme leaves the country on the scheme it already had.

```ini title="rules.ini"
[GDI]
Color=Gold
```

Each house takes this scheme as it is created and recolors everything it owns with it. Two things then move it. A campaign scenario may override the house's copy with a [`Color=`](/keys/color/#scope-house-per-scenario) in its own house record, which leaves the country's copy alone. A skirmish or multiplayer game overwrites *both* copies with the color the player picked in the lobby, so the country's own setting survives only into a campaign.

That second overwrite is why the country's copy is worth knowing about at all. Several places color their text from the country rather than from the house, the alliance and connection messages among them, and in a lobby game those follow whichever house of that country was set up last.
