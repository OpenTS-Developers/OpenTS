---
title: Read the interface's own PCX art in UI documents
category: internal
release: 0.2.0
targets:
- type: system
  id: ui-files
  effect: changed
credit:
- ZivDero
---

A UI document can name a PCX image, which is what the game's own dialog art is stored as, alongside the PNG and TGA it already read. The file is found through the game's search, so art inside a mix archive works the same as art beside the executable. Pure magenta is the color key this art is drawn with, and those pixels come through clear.

Art the player does not have no longer stops a screen from opening. A missing image leaves nothing where it would have drawn; an image that is present and will not decode still sends the screen to its Win32 dialog, because a document that names a broken file is asking for something it cannot have.

No art ships with OpenTS. Nothing uses this yet: the screens that draw with it follow.
