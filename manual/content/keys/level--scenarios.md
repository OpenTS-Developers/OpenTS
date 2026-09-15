---
key: Level
scope: scenarios
label: Height brightness
see_also: [Ground, IonLevel, "system:ion-storms"]
when_omitted:
  kind: value
  value: "0"
  note: The engine starts each scenario with about a sixtieth of full light here. When the key is absent, the fallback divides two whole numbers and truncates to zero, so the engine stores zero rather than that fraction.
---

```ini title="map file"
[Lighting]
Level=.016
```

Cell brightness is the ambient level, plus this figure for each height level the cell stands above the ground, minus [`Ground`](/keys/ground/). The same figure also scales the height bonus drawn onto aircraft and onto elevated units and infantry. At `0` the map is shaded flat, with no cliff or hill standing out from the ground around it.

[`IonLevel`](/keys/ionlevel/) replaces this value for the length of [an ion storm](/systems/ion-storms/#lighting), and it truncates on omission in the same way.
