---
title: Name the track that is playing
category: feature
release: 0.2.0
targets:
- type: key
  id: NowPlaying
  effect: added
- type: command
  id: fixed:next-track
  effect: added
- type: command
  id: fixed:previous-track
  effect: added
credit:
- fjlaubscher
---

`NowPlaying=yes` under `[siliconsun]` in `SUN.INI` prints the playing track's title at the bottom left of the battlefield for the same time as an alert. While that setting is on, the media next-track and previous-track keys run the next track and previous track commands. The line is on by default.
