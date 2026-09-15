---
key: Invisible
summary: Whether the object is drawn for the player that owns it alone and kept off the radar entirely.
see_also: [InvisibleInGame, "system:cloaking"]
when_omitted:
  kind: value
  value: "no"
---

An `Invisible=yes` object is drawn normally for the machine whose own player owns it and not drawn at all on any other, whatever its cloak state. It is never plotted on the radar, and it is skipped by the tooltip, the cursor and click-selection on every machine but its owner's.

A BuildingType set to [`InvisibleInGame=yes`](/keys/invisibleingame/) has this key forced on afterwards. That setting overrides an `Invisible=no` written in the same section.

:::danger[Firing decisions come out differently on each machine]
The test that stops an object from shooting at something completely out of sight asks whether the target is drawn for the local player, not for the firing house. An `Invisible=yes` object is therefore a legal target on its owner's machine and an illegal one everywhere else. The same attack goes ahead in one player's copy of the match and is refused in another's.
:::
