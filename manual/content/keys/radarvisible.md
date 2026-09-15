---
key: RadarVisible
summary: Whether the object is plotted on the radar map under every condition.
see_also: [Invisible, "system:cloaking"]
when_omitted:
  kind: value
  value: "no"
---

`RadarVisible=yes` answers the [radar test](/systems/cloaking/#on-the-radar) before it reaches any of its conditions. Shroud, fog of war, cloaking, tunneling, the radar-invisible ability, and whether the player has ever discovered the object all stop mattering. An object of an enemy house is plotted from the moment it exists, before the player has ever laid eyes on it. It stays plotted wherever it goes.

Only [`Invisible=yes`](/keys/invisible/) outranks it, because that is tested first and keeps the object off the radar for every house. A BuildingType set to [`InvisibleInGame=yes`](/keys/invisibleingame/) has this key forced back to `no` afterwards.
