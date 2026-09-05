---
key: MinVolume
scope: sounds
label: Distance floor
see_also: [Volume, Range, Type]
when_omitted:
  kind: value
  value: "0"
---

The loudness a sound with `Type=GLOBAL` never falls below however far its place is from the view, as a fraction of full loudness; a value above 1 is read as a percentage, as for [`Volume=`](/keys/volume/). Without `GLOBAL` in the sound's [`Type=`](/keys/type/) the key has no effect and the sound fades to silence beyond its [`Range=`](/keys/range/).

```ini title="sound01.ini"
[BIGBLAST]
Type=GLOBAL
MinVolume=0.3
```
