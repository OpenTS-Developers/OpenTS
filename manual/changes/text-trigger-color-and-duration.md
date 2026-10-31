---
title: Choose a text trigger's color and display time
category: feature
release: 0.2.0
targets:
- type: action
  id: TACTION_TEXT_TRIGGER
  effect: changed
credit:
- JoyfulShush
- ZivDero
---

A Text Trigger action now takes its color and display duration from the first two of the four fields after the text line number. The first selects a color from `[Colors]`, counted from `0`; the second gives the display time in seconds. A map that leaves both fields at `0` keeps the first default color and `MessageDelay`.

Credits to ZivDero for their original Vinifera implementation for this feature.
