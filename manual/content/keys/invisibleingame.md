---
key: InvisibleInGame
summary: Whether the structure is drawn at all, offered to the player, or plotted on the radar.
see_also: [Invisible, "system:cloaking"]
when_omitted:
  kind: value
  value: "no"
---

The engine skips the structure's shape when it draws, so it is never drawn for any house, its owner included. That is what separates this from [`Invisible=yes`](/keys/invisible/), where the owner still sees it. It offers the player no action and no other object offers one against it, the cursor passes over it, and an [EM pulse](/systems/emp-pulse/#what-a-pulse-reaches) skips it when the cell sweep reaches its center.

Setting it forces `Invisible=yes` and [`RadarVisible=no`](/keys/radarvisible/) onto the same type. Both are read before it, so it overrides whatever those two keys set in the same section.
