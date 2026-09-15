---
key: SuspendVoice
summary: The EVA line spoken when the cameo of a superweapon whose timer is stopped is clicked.
see_also: ["system:superweapons"]
when_omitted:
  kind: value
  value: none
---

This line is spoken when a click lands on a cameo the weapon cannot be fired from and the charge timer is stopped rather than running. [`ImpatientVoice=`](/keys/impatientvoice/) is the line for the same click while the timer runs. Nothing speaks it at the moment a weapon is [suspended](/systems/power/#superweapons); it is only ever a reply to a click. Because a [`ManualControl=yes`](/keys/manualcontrol/) weapon's timer is stopped between firings, this is the line such a weapon gives while it is uncharged. An unrecognized speech name is not rejected; it leaves the weapon with no line.
