---
title: Add the RmlUi shell and its test document
category: internal
release: 0.2.0
targets:
  - type: command
    id: fixed:debug-ui-test-document
    effect: added
credit: [ZivDero]
---

The engine gains a UI shell that draws RmlUi documents through the renderer over the presented frame, reads them through the game's own file search, and takes their input ahead of the game. Nothing player-facing uses it yet: the first migrated screens follow in later changes.

A `ui` directory of documents, styles, and the Open Sans font now ships beside the executable. A Debug build with the debug keys armed shows a test document on F9; a Release build carries the shell but shows nothing.
