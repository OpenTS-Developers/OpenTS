---
key: UpgradeEliteSound
summary: The sound played when one of the local player's objects is promoted to elite.
see_also: [UpgradeVeteranSound, VoxUnitPromoted, EliteFlashTimer, "system:veterancy"]
when_omitted:
  kind: value
  value: none
---

```ini title="rules.ini"
[AudioVisual]
UpgradeEliteSound=PROMOTE2 ; a sound ID registered in SOUND.INI
```

The sound plays when an object of a house the local player controls rises to elite rank in an announced promotion. It plays from the object's position, so it fades and pans with the object on screen.

A rise to elite from any lower rank uses this sound, including one that skips the veteran rank; [`UpgradeVeteranSound`](/keys/upgradeveteransound/) is not played as well. A rise from below rookie to rookie plays nothing. [Promotion feedback](/systems/veterancy/#promotion-feedback) lists the announcements that carry this sound and the ones that stay silent.

An unset sound name, or a name that matches no registered sound, plays nothing.
