---
key: RadialColor
scope: aircrafttype
label: Ring color
when_omitted:
  kind: value
  value: 0,0,0
---

An aircraft never draws a [radius ring](/keys/hasradialindicator/), so an AircraftType stores this color and nothing in the engine reads it.

```ini title="rules.ini"
[MYCLOAK] ; example BuildingType
CloakGenerator=yes
CloakRadiusInCells=12
HasRadialIndicator=yes
RadialColor=0,168,240
```

Only a structure that draws a ring reads the value: the ellipse and the four spokes sweeping around it are drawn in this color. The engine default is black, and black is drawn rather than skipped, so a type that asks for a ring without naming a color gets a black one.
