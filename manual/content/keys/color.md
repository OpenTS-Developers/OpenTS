---
key: Color
summary: Color scheme for a projectile, house, or Tiberium type; the multiplayer setting holds the index of the player's preferred color.
---

`Color=` is read in five scopes. Four of them take a color scheme name and one takes a number, so the file the setting belongs in decides what a valid value looks like.

The four name scopes, each read from the section the type or house owns:

- [Voxel projectile remap](/keys/color/#scope-bullettype): the bullet type's own section in the rules. It recolors the type's voxel model in flight.
- [Country color](/keys/color/#scope-housetype): the country's section in the rules. It colors everything the country owns, and it survives into a campaign only.
- [Scenario house color](/keys/color/#scope-house-per-scenario): the scenario map's own house record. It recolors that one house for that one mission.
- [Tiberium remap](/keys/color/#scope-tiberium): the Tiberium type's section in the rules. It recolors the overlay and the animations it spawns.

The name each of these holds is matched against the schemes the rules declare in `[Colors]`: one entry per scheme, giving the hue, saturation and value its palette remap is built from. Every declaration builds two variants, a flat one and a full intensity ramp, and the flat one is the single-intensity scheme the name match passes over.

The number scope is the [preferred multiplayer color](/keys/color/#scope-multiplayer-settings). It lives in `[MultiPlayer]` of the player's own settings file, `SUN.INI`, and holds a position in the lobby's eight-entry color list rather than any name the rules know.
