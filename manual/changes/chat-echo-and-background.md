---
title: Show your own chat line and draw a background behind the message list
category: feature
release: 0.2.0
targets:
- type: key
  id: TextBackgroundColor
  effect: added
credit: [ZivDero, Iran, dkeeton, CCHyper, tomsons26]
---

Your own chat line now appears on your screen as it is sent, as the recipients see it, tagged
with who it went to. `TextBackgroundColor` under `[Options]` in `sun.ini` draws a colour behind
every glyph of the message list and its editor: the default `12` is the black box the CnCNet
client's chat background option writes, and `0` draws none. A name is now followed by a space
before the text.

Iran wrote the echo, from the code he wrote for Red Alert, and dkeeton the message background.
CCHyper and tomsons26 are credited for the Vinifera versions of both.
