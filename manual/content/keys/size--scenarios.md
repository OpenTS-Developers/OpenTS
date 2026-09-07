---
key: Size
scope: scenarios
label: Playfield rectangle
see_also: ["system:map-visibility", LocalSize]
when_omitted:
  kind: value
  value: "1,1,50,50"
---

The rectangle is read as four numbers; only its width and height are kept, since the origin is forced to zero. Those two figures define the playfield that every reveal, every path and every radar query is bounded by, and reading them rebuilds every cell from scratch — which is why a scenario always opens with its whole map shrouded and fogged.

The `[Map]` section is not layered like the rules files: this key is read from the scenario alone, with the literal fallback above when it is absent.

```ini title="map file"
[Map]
Size=0,0,120,120
LocalSize=2,4,116,112
```
