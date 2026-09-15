---
key: ToOverlay
summary: The cell overlay a wall structure turns into when it is placed.
see_also: ["system:walls-and-gates", "Wall"]
when_omitted:
  kind: value
  value: none
---

```ini title="art.ini"
[GAWALL]
ToOverlay=GAWALL ; the OverlayType of the same name
```

A [`Wall=yes`](/keys/wall/#scope-buildingtype) BuildingType [converts into this overlay](/systems/walls-and-gates/#from-structure-to-overlay) on reaching the map and deletes itself. The named overlay also decides three other things:

- whether placing the type triggers the [automatic gap fill](/systems/walls-and-gates/#filling-the-gap-to-the-next-wall), which is switched on by the named overlay being a wall rather than by the structure's own flag;
- which existing damaged segments the type may be placed on top of;
- through the first BuildingType in the rules that names a given overlay, whether that overlay can be sold at all.

A name that matches no registered OverlayType registers a new, unconfigured overlay under that name rather than failing. The new overlay has no artwork, is not treated as a wall, and its cells count as clear terrain. The values `none` and `<none>` resolve to nothing at all, exactly as if the key had been left out.

:::danger[A wall structure with no overlay named crashes on placement]
The conversion reads this setting without checking it. A BuildingType with `Wall=yes` and either no `ToOverlay=` or `ToOverlay=none` crashes the game the first time one of them is placed. A computer house does not get that far: a base plan holding such a type reads the same empty setting while the plan is still being laid, before anything reaches a cell.
:::
