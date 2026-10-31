---
key: CloakSound
scope: global-rules
label: Rules-wide cloak sound
when_omitted:
  kind: value
  value: none
---

```ini title="rules.ini"
[AudioVisual]
CloakSound=CLOAK5 ; the shipped sound, defined in SOUND.INI
```

The rules-wide sound plays at an object's position as it starts to hide and as it starts to reappear. That includes objects a [cloaking field](/systems/cloaking/#cloaking-fields) hides: vehicles, infantrymen and aircraft on a covered cell, and structures whose center cell the field covers.

A name that matches no [registered sound](/formats/sound-ini/) is ignored; with no sound already set, nothing plays.

A type can replace it with sounds of its own: [`CloakSound`](/keys/cloaksound/#scope-aircrafttype) plays as that type hides, and [`UncloakSound`](/keys/uncloaksound/) plays as it reappears.
