---
key: InitialVeteran
summary: Creates the randomly chosen starting units of a skirmish or multiplayer match at elite rank.
see_also: ["system:veterancy", "system:starting-forces"]
when_omitted:
  kind: value
  value: "no"
  note: Each scenario starts with this built-in default.
---

```ini title="map file"
[SpecialFlags]
InitialVeteran=yes
```

`InitialVeteran=yes` makes every house's randomly chosen starting vehicles and infantry elite as they are placed. [Starting forces](/systems/starting-forces/#spending-the-budget) owns how those objects are chosen. The [base unit](/systems/starting-forces/#the-base-unit) given to each house in a game with bases is created separately and starts as a rookie.

The entry works in skirmish and multiplayer games. Those games replace the map's flags with the game options, but only after the starting forces are placed, so this entry is still taken from the map. A single-player mission places no starting forces, so the entry has no effect there.
