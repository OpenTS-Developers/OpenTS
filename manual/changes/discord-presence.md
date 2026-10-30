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

`DiscordPresence=yes` under `[siliconsun]` in `SUN.INI` shows the current activity on the local Discord desktop client: the menus, a skirmish, a campaign mission, or a multiplayer game, with the map name, GDI or Nod, the skirmish AI count, and the elapsed time. It is on by default. `no` does not contact Discord.
