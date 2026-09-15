---
key: ImpatientVoice
summary: The EVA line spoken when the cameo of a still-charging superweapon is clicked.
see_also: ["system:superweapons"]
when_omitted:
  kind: value
  value: none
---

Clicking a cameo the weapon cannot be fired from picks between this line and [`SuspendVoice=`](/keys/suspendvoice/) on whether the charge timer is running: a running timer gives this one. A [`ManualControl=yes`](/keys/manualcontrol/) weapon has its timer stopped between firings, so it gives the suspend line instead, whatever is set here. An unrecognized speech name is not rejected; it leaves the weapon with no line.
