---
key: Color
scope: house-per-scenario
label: Scenario house color
see_also: [PlayerControl, Credits, Edge]
when_omitted:
  kind: inherited
  note: The color scheme the house's country sets, from its own Color= in the rules.
---

The value is a color scheme name, matched against the loaded schemes without regard to letter case. A scheme with only one intensity level is skipped during that match, and a name matching no remaining scheme leaves the house on the scheme it already had.

```ini title="scenario map file"
[Nod] ; a house record in the scenario's own house list
Color=DarkRed
```

The scheme recolors every unit, structure and infantryman the house owns. It also sets the house's dot on the radar, the chat and warning lines it is given, and its column on the score screen. Changing it here overrides the country's own [`Color=`](/keys/color/#scope-housetype) for this scenario only, and it leaves the country's setting alone. Anything drawn from the country rather than from the house keeps the original scheme.

Only a campaign mission reads its house records, so this is a campaign setting. A skirmish or multiplayer house takes its scheme from the color each player picked in the lobby instead, and that assignment overwrites the country's scheme as well as the house's.
