---
key: UpgradeVeteranSound
summary: The sound played when one of the local player's objects is promoted to veteran.
see_also: [UpgradeEliteSound, VoxUnitPromoted, VeteranFlashTimer, "system:veterancy"]
when_omitted:
  kind: value
  value: none
---

```ini title="rules.ini"
[AudioVisual]
UpgradeVeteranSound=PROMOTE1 ; a sound ID registered in SOUND.INI
```

The sound plays when an object of a house the local player controls rises to veteran rank in an announced promotion. It plays from the object's position, so it fades and pans with the object on screen.

A rise to elite uses [`UpgradeEliteSound`](/keys/upgradeelitesound/) instead, including a rise that skips the veteran rank. A rise from below rookie to rookie plays nothing. [Promotion feedback](/systems/veterancy/#promotion-feedback) lists the announcements that carry this sound and the ones that stay silent.

An unset sound name, or a name that matches no registered sound, plays nothing.
