---
key: ScreenTextColor
summary: The color of the mission-end caption for a side.
when_omitted:
  kind: value
  value: "255,255,255"
---

The color used for the text caption that announces the outcome of a mission or match: the "Mission Accomplished" or "Mission Failed" caption in a campaign, and the victory or defeat text in a skirmish or multiplayer game.

```ini title="rules.ini"
[GDI]
ScreenTextColor=135,180,255 ; light blue
```

The color comes from the side of the local player's house. In a skirmish or multiplayer game, each player sees the text in the color of their own side.
