---
title: Take the presented side from the player's country
category: fix
release: 0.2.0
targets:
- type: key
  id: Player
  scope: scenarios-2
  effect: changed
- type: key
  id: SpeechSide
  effect: changed
credit: [ZivDero]
---

A mission's art, interface and voices now follow the side of the country `Player=` names, and a
lobby game's follow the side of the country the player chose. The choice was a text comparison
against `GDI`: anything else, a third country included, was presented as Nod, and a third side
could be reached only by naming it in `SpeechSide=`, and only for voices. A side without archives
of its own is presented with the first side's instead of abandoning the load. A saved game now
records the player's country and its side in place of a flag, so a save from an earlier
development build of this version is misread.
