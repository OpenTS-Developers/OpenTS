---
key: Limit
scope: sounds
label: Copies at once
see_also: [Priority, Control, Channels]
when_omitted:
  kind: value
  value: "3"
---

How many copies of the sound may play at once; `0` allows any number. When the limit is reached, a new copy louder than the quietest one playing takes its place, and a new copy no louder is refused. With `INTERRUPT` in the sound's [`Control=`](/keys/control/) a copy as loud as the quietest takes the place of the oldest instead of being refused; with `QUEUE` a refused copy waits up to two seconds for a place.

```ini title="sound01.ini"
[GUN5]
Limit=2
```

The limit is applied before the voice budget of [`Channels=`](/keys/channels/) and only among copies of the same sound; [`Priority=`](/keys/priority/) decides between different sounds.
