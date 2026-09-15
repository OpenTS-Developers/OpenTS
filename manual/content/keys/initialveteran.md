---
key: InitialVeteran
summary: Creates the randomly chosen starting units of a skirmish or multiplayer match at elite rank.
see_also: ["system:veterancy", "system:starting-forces"]
when_omitted:
  kind: value
  value: "no"
  note: The special options are initialized with this built-in default when the game starts.
---

Every scenario's map file is parsed for the flag, but the game reads it in one place: [the pass that places each house's starting forces](/systems/starting-forces/#spending-the-budget). That pass runs only outside campaign play, so a campaign scenario with the setting is unaffected. Every other game type replaces the scenario's whole set of flags with the set the game options hold, but only after that pass has run, so the map's value is the one it reads.

```ini title="map file"
[SpecialFlags]
InitialVeteran=yes
```

Only the units and infantry drawn from the random starting selection are promoted. The construction vehicle placed when the match is set up with bases is created separately and stays a rookie.
