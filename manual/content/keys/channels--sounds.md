---
key: Channels
scope: sounds
label: Sound effect voices
see_also: [Priority, Limit]
when_omitted:
  kind: value
  value: "16"
---

The number of sound effects that may play at once, from 4 to 32, read from `[General]` in `SOUND.INI` or `SOUND01.INI`. Music, speech and movie sound are not counted. When every voice is taken, a new sound displaces the playing sound with the lowest [`Priority=`](/keys/priority/), the quietest among equals, and only when it outranks it.

```ini title="sound01.ini"
[General]
Channels=24
```
