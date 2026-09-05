---
key: Sounds
scope: sounds
label: Sample list
see_also: [Attack, Decay, Control]
when_omitted:
  kind: computed
  note: The one sample named like the section.
---

The samples the sound plays, named without an extension and separated by spaces or commas, up to thirty-two of them. The first [`Attack=`](/keys/attack/) names are attack samples and the last [`Decay=`](/keys/decay/) names are decay samples; the rest are the body, which [`Control=`](/keys/control/) says how to play. Without the key the sound plays the one sample named like its section.

```ini title="sound01.ini"
[MYLOOP]
Sounds=LOOPIN LOOPBODY1 LOOPBODY2 LOOPOUT
Control=LOOP RANDOM ATTACK DECAY
```

Each name is looked up when the sound first plays, as `.WAV`, `.OGG`, `.FLAC`, `.MP3` and then `.AUD`, through the file layer that sees loose files and every mounted archive. A name that resolves to nothing is left out of that play, and the next body sample stands in for a missing one, so the sound still plays as long as one sample is found.
