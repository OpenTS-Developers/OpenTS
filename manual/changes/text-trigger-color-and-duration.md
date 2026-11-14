---
title: Choose a text trigger's color and display time
category: feature
release: 0.3.0
targets:
- type: action
  id: TACTION_TEXT_TRIGGER
  effect: changed
credit:
- JoyfulShush
- ZivDero
---

A Text Trigger action now takes its color and display duration from the first two of the four fields after the text line number. The first selects a color by its position in the rules' `[Colors]`, counted from `0`; the second gives the display time in seconds. A map that leaves both fields at `0` keeps the first default color and `MessageDelay`.

ZivDero is credited for the Vinifera implementation this follows.
