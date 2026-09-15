---
key: IonLevel
scope: scenarios
label: Scenario lighting
see_also: [Level, IonGround, "system:ion-storms"]
when_omitted:
  kind: context-dependent
  note: This scenario's Lighting Level value truncated to a whole number, because the fallback divides two whole numbers. Every Level value below 1 therefore yields 0.
---

```ini title="map file"
[Lighting]
Level=.016
IonLevel=.016
```

While a storm runs, this figure replaces [`Level`](/keys/level/#scope-scenarios) as the brightness added for each height level a cell stands above the ground. It also scales the height bonus drawn onto aircraft and onto elevated units and infantry.

:::danger[The fallback cannot reproduce an ordinary level value]
Leaving this key out does not copy `Level` across: the fallback is a whole-number division that truncates any fraction below `1` to `0`. A map that omits it is shaded flat for the length of every storm, with cliffs and hills no brighter than the ground beside them. State the value outright to keep the shading.
:::
