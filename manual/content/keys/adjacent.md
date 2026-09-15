---
key: Adjacent
summary: How far this building searches for an eligible anchor while it is being placed.
see_also: [BaseNormal, "system:base-adjacency"]
when_omitted:
  kind: value
  value: "3"
---

```ini title="rules.ini"
[GAPOWR]
Adjacent=5
```

While the player places `GAPOWR`, this value controls how far the pending foundation searches for an eligible [anchor](/systems/base-adjacency/): a building with [`BaseNormal=yes`](/keys/basenormal/) that belongs to the placing house or to an ally the match admits. The value does not define a radius projected by an already placed `GAPOWR`.

:::note[Zero still allows touching placement]
The scan adds one cell to the stored value. With `Adjacent=0`, an eligible anchor may still be found directly beside the pending foundation.
:::
