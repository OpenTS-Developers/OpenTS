---
title: Add hotkeys to change game speed
category: feature
release: 0.2.0
targets:
- type: command
  id: IncreaseGameSpeed
  effect: added
- type: command
  id: DecreaseGameSpeed
  effect: added
credit:
- JoyfulShush
---

Two new commands change the game speed during play, one step at a time. Increase Game Speed moves toward the fastest value and Decrease Game Speed toward the slowest. Neither command has a default key, and both do nothing in a multiplayer game.
