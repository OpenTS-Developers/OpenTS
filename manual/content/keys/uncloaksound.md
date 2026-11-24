---
key: UncloakSound
summary: The sound an object of this type plays as it starts to reappear.
see_also: [CloakSound]
when_omitted:
  kind: inherited
  note: The object plays the rules-wide CloakSound instead.
---

```ini title="rules.ini"
[STNK]
UncloakSound=MYREVEAL ; a sound ID registered in SOUND.INI
```

An object of this type plays this sound at its position as it starts to reappear, in place of the [rules-wide `CloakSound`](/keys/cloaksound/#scope-global-rules). No rules-wide uncloak sound exists, so a type that doesn't specify this sound plays the rules-wide `CloakSound` as it reappears.

The setting covers reappearing only. The object still plays its own [`CloakSound`](/keys/cloaksound/#scope-aircrafttype), or the rules-wide `CloakSound`, as it starts to hide.

A name that matches no [registered sound](/formats/sound-ini/) is ignored. A type that still has no sound of its own then plays the rules-wide `CloakSound`.
