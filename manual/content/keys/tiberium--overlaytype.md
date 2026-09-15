---
key: Tiberium
scope: overlaytype
label: Tiberium overlay
see_also: ["system:tiberium", "ChainReaction", "Image"]
when_omitted:
  kind: value
  value: "no"
---

```ini title="rules.ini"
[MYTIB] ; an OverlayType registered in [OverlayTypes]
Tiberium=yes
```

Setting the flag rewrites two of the overlay type's own settings once its section has been read. Its armor becomes wood even if the section sets [`Armor=`](/keys/armor/) itself, and a [`Land=Clear`](/keys/land/) overlay is promoted to the `Tiberium` [land type](/reference/enums/land-type/). That land type is the ground condition every harvesting test reads. An overlay left on another land type keeps it and is never harvested.

The flag also makes the overlay eligible for the reverse lookup that decides which Tiberium in the rules' [`[Tiberiums]` list](/formats/rules-registries/) a cell belongs to. An overlay with the flag that falls outside every type's overlay range is read as the first type in that list. The bails a harvester lifts there are converted at that type's [`Value=`](/keys/value/).
