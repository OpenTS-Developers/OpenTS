---
key: VoxUnitPromoted
summary: The EVA line requested when one of the local player's objects is promoted to veteran or elite.
see_also: [UpgradeVeteranSound, UpgradeEliteSound, "system:veterancy", "system:eva-speech"]
when_omitted:
  kind: value
  value: none
---

The line is requested when an object of a house the local player controls rises to veteran or elite rank in an announced promotion. One line covers both ranks. [Promotion feedback](/systems/veterancy/#promotion-feedback) lists the announcements that request it and the ones that stay silent.

It joins the speech queue and follows the ordering, volume, and silencing rules in [EVA speech](/systems/eva-speech/). A rise from below rookie to rookie requests nothing.

An unrecognized speech name requests no line.
