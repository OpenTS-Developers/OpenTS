---
title: Give an Internet game its own in-game options menu
category: fix
release: 0.2.0
targets:
- type: system
  id: ui-files
  effect: changed
credit:
- ZivDero
---

The in-game options menu of an Internet game shows the game speed bar and the connection reading its dialog carries, in place of the three-button menu of a local network game. Changing the speed sends it to the other players as it always did, and Save and Load ask the match for a save or a load rather than opening a file list in the middle of a frame.

The bar names the speed it stands at as soon as the menu opens, where the dialog left "Faster" standing until the bar was first moved.
