---
key: Priority
scope: sounds
label: Sound playback priority
see_also: [Limit, Volume, Channels]
when_omitted:
  kind: value
  value: "10"
---

A number from 0 to 255, or one of `LOWEST`, `LOW`, `NORMAL`, `HIGH` and `CRITICAL` for 0, 10, 50, 100 and 255. Up to [`Channels=`](/keys/channels/) sound effects play at once. A new sound takes a free voice whatever its priority. With every voice busy it takes the voice of the playing sound with the lowest priority, the quietest among equals. It wins only when its own priority is higher, or equal and the playing sound more than a tenth quieter than the new one. A sound that qualifies for no voice is dropped without playing. If its `Control=` is `QUEUE`, it waits up to two seconds for one.

```ini title="sound01.ini"
[EXPLOLG1]
Priority=50

[MYALERT]
Priority=CRITICAL
```

The figure is compared as written; the loudness a sound is played at no longer lowers it. [`Limit=`](/keys/limit/) is applied first and separately, among copies of the same sound.
