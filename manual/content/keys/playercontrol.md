---
key: PlayerControl
summary: Marks a scenario's house as the one the local player is playing, which in a campaign decides the difficulty slot it is given and whether it is spoken to.
see_also: ["system:difficulty"]
when_omitted:
  kind: value
  value: "no"
---

A campaign scenario decides what counts as the local player's house by asking each house two questions: whether it is flagged as human, and whether it has this setting. Either one is enough. Outside a campaign the question is answered by identity instead: only the house the local machine is actually playing counts, and this setting is ignored there.

```ini title="scenario map file"
[GDI] ; a house record in the scenario's own house list
PlayerControl=yes
Edge=North
```

What that answer governs is everything the engine does *for* a player rather than *to* a house. It decides the difficulty slot the house is given, the EVA lines it hears, the radar events drawn for it, and the sidebar and placement feedback it is given. [Difficulty settings and handicaps](/systems/difficulty/#from-the-setting-to-a-slot) covers that first half, which is where a missing flag is felt hardest.

Nothing clears the setting in a campaign. Only a multiplayer game clears it, when the computer takes over the house of a player who has left.

:::caution[Setting it on more than one house is allowed]
Nothing restricts the flag to a single house, and each house with it is treated as player-controlled in its own right. When difficulty slots are assigned, every one of them is given the player's slot rather than the computer's.
:::
