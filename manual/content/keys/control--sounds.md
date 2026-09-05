---
key: Control
scope: sounds
label: How the samples are played
see_also: [Sounds, Loop, Delay, Attack, Decay, Limit]
when_omitted:
  kind: value
  value: NORMAL
---

Flags, separated by spaces or commas, saying how the samples of [`Sounds=`](/keys/sounds/) are put together into one play. The values follow Yuri's Revenge, with `SEQUENTIAL` and `QUEUE` from Vinifera.

- `NORMAL`: one body sample, played once.
- `LOOP`: the body repeats, [`Loop=`](/keys/loop/) times or until the game ends the sound, with [`Delay=`](/keys/delay/) between cycles.
- `RANDOM`: the body is one sample drawn at random, and the same one for every cycle of that play.
- `SEQUENTIAL`: the body is the next sample in turn each time the sound is played.
- `ALL`: the body is every body sample, in order. `ALL` wins over `RANDOM`, which wins over `SEQUENTIAL`.
- `PREDELAY`: the `Delay=` silence comes once, before the first sample.
- `INTERRUPT`: when [`Limit=`](/keys/limit/) is reached by a copy as loud as the new one, the oldest gives way instead of the new one being refused.
- `ATTACK`, `DECAY`: the list has attack or decay samples; [`Attack=`](/keys/attack/) and [`Decay=`](/keys/decay/) say how many, and one each.
- `QUEUE`: a play refused for want of a voice, or by `Limit=`, waits up to two seconds for one instead of being dropped.
- `AMBIENT` is read and kept but decides nothing.

```ini title="sound01.ini"
[MYLOOP]
Sounds=LOOPIN LOOPBODY1 LOOPBODY2 LOOPOUT
Control=LOOP RANDOM ATTACK DECAY
```

A flag the engine does not know is ignored.
