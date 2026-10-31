---
key: CloakSound
scope: aircrafttype
label: Type cloak sound
see_also: [UncloakSound]
when_omitted:
  kind: inherited
  note: The object plays the rules-wide CloakSound instead.
---

```ini title="rules.ini"
[STNK]
CloakSound=CLOAKHIDE ; a sound ID registered in SOUND.INI
```

An object of this type plays this sound at its position as it starts to hide, in place of the [rules-wide `CloakSound`](/keys/cloaksound/#scope-global-rules). A [cloaking field](/systems/cloaking/#cloaking-fields) that hides an object plays this sound too.

The setting covers hiding only. The same object plays its own [`UncloakSound`](/keys/uncloaksound/), or the rules-wide `CloakSound`,  as it starts to reappear.

A name that matches no [registered sound](/formats/sound-ini/) is ignored.
