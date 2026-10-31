---
title: Show the game on Discord
category: feature
release: 0.2.0
targets:
- type: key
  id: DiscordPresence
  effect: added
credit:
- fjlaubscher
---

`DiscordPresence=yes` under `[siliconsun]` in `SUN.INI` shows the current activity on the local Discord desktop client. In a game the first line names Tiberian Sun or Tiberian Sun - Firestorm and the match, such as Campaign, 2v2, or 1v1v1, and the second line names the side and the scenario. It is on by default. `no` does not contact Discord.
