---
key: Player
scope: scenarios-2
label: Presentation side
see_also: [SpeechSide, Theater]
when_omitted:
  kind: value
  value: "GDI"
---

```ini title="map file"
[Basic]
Player=Nod
```

Long before the house is resolved, the same assignment is read as a country name, matched against the country IDs the rules declare without regard to case, and the side of that country selects the mission's artwork, interface and sidebar and seeds the voice set before [`SpeechSide`](/keys/speechside/) gets its chance to change it. A value naming no country is presented as the first country. Outside a campaign the country comes from the lobby instead.

A side with no artwork archives of its own is presented with the first side's; the load is given up only when the first side's archives cannot be mounted either. The same applies to the voice set prepared immediately afterwards.
