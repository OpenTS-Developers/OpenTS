---
key: CellTagsIgnoreStealth
summary: Whether a fully cloaked infantryman or vehicle triggers cell tags as it moves.
see_also: ["system:trigger-springing", "system:cloaking"]
when_omitted:
  kind: value
  value: "yes"
---

`CellTagsIgnoreStealth=no` lets a fully cloaked infantryman or vehicle trigger the same cell tag events as a visible object when it finishes moving into a cell: cell entry, both line crossings and zone entry. `CellTagsIgnoreStealth=yes` keeps a fully cloaked object from triggering any of them.

An object that is still cloaking or uncloaking is not fully cloaked, so it triggers these events whether the key is `yes` or `no`.

The setting does not change the events an object's own tag receives, such as when infantry arrives at a structure or vehicle.

```ini title="rules.ini"
[General]
CellTagsIgnoreStealth=no  ; fully cloaked infantry and vehicles trigger cell tags
```
