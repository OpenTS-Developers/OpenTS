---
format_id: sound-ini
title: SOUND.INI
summary: Registers sound IDs and defines each one's samples, loudness, priority, range, and playback behavior.
kind: file
filenames:
  - SOUND.INI
  - SOUND01.INI
key_scopes:
  - file: sound01.ini
    section:
      kind: identifier
      source: sound
related:
  - { type: format, id: aud }
source_files:
  - code/init.cpp
  - code/sndtype.cpp
  - code/voc.cpp
---

Startup reads `SOUND.INI` and `SOUND01.INI` into one database. Whichever files are present are read, the expansion's file over the base one. A sound both files name comes from `SOUND01.INI`, and one only `SOUND.INI` names is kept. Either file alone is enough, and initialization stops only when neither can be read. Whether Firestorm is installed does not decide which file is read. The same rule applies to `[General]` and `[Defaults]`: a section in the expansion's file replaces the base one.

`[SoundList]` values register sound IDs. Each ID names a section; a section that does not exist, or has no keys, gives a sound that plays the sample of the same name with the defaults.

```ini title="SOUND.INI"
[General]
Channels=16

[Defaults]
Priority=NORMAL
Limit=3

[SoundList]
0=MYALERT
1=MYLOOP

[MYALERT]
Priority=100

[MYLOOP]
Sounds=LOOPIN LOOPBODY1 LOOPBODY2 LOOPOUT
Control=LOOP RANDOM ATTACK DECAY
Delay=250 750
Range=20
Limit=1
```

The whole sound list is discarded and rebuilt each time the file is read, and the entries are registered in the order the section lists them. An ID that is already registered is filled in again rather than added a second time, so naming the same sound twice leaves one sound rather than two.

## Samples

A sound no longer holds a sample from startup. Each name in its list is looked up when the sound first plays, through the ordinary file layer, so a loose file in the game directory and a member of any mounted archive both serve, and the loose file wins. The formats tried, in order, are `.WAV`, `.OGG`, `.FLAC`, `.MP3` and `.AUD`. A decoded sample stays in memory while the sound plays and for as long as the memory is not needed for another. A name that resolves to nothing is dropped from the list at play time, and a sound whose whole list is missing plays nothing.

## `[General]`

`Channels=` is the number of sound effects that may play at once, from 4 to 32; 16 when the section or the key is absent. Music, speech and movie sound are outside it. When it is full, a new sound takes the voice of the sound with the lowest [`Priority=`](/keys/priority/), and among equals the quietest, but only when it outranks it.

## `[Defaults]`

Every key of a sound section may appear here and becomes the value a sound section omits. Without the section, the engine's own defaults apply: those are the values listed under each key below.

## Sound sections

| Key | Default | Meaning |
| --- | --- | --- |
| [`Sounds=`](/keys/sounds/) | the section name | The samples, without extension, separated by spaces or commas; up to 32. The first `Attack=` names are attack samples and the last `Decay=` names decay samples. |
| [`Priority=`](/keys/priority/) | `10` | A number from 0 to 255, or `LOWEST`, `LOW`, `NORMAL`, `HIGH` or `CRITICAL` for 0, 10, 50, 100 or 255. |
| [`Volume=`](/keys/volume/) | `1.0` | A fraction of full loudness. A value above 1 is read as a percentage. |
| [`MinVolume=`](/keys/minvolume/) | `0` | The floor a `GLOBAL` sound never drops below with distance, in the same units. |
| [`Range=`](/keys/range/) | `28` | Cells from the edge of the view over which the sound fades to nothing. |
| [`Limit=`](/keys/limit/) | `3` | How many of this sound may play at once; 0 for no limit. |
| [`Loop=`](/keys/loop/) | `0` | With `LOOP`, how many times the body plays; 0 plays it until the sound is ended. `LoopLimit=` is accepted as well. |
| [`Delay=`](/keys/delay/) | `0` | One or two numbers, the silence between loop cycles in milliseconds, drawn at random from the pair. A number with a decimal point is in seconds. With `PREDELAY` the silence comes once, before the first sample. |
| [`FShift=`](/keys/fshift/) | `0` | One or two percentages of pitch shift, drawn once per play. A single value spans both ways. |
| [`VShift=`](/keys/vshift/) | `0` | One or two percentages of volume change, drawn once per play. A single value quietens by up to that much; a pair is a signed range. |
| [`Type=`](/keys/type/) | `SCREEN` | Where the sound is heard from; see below. |
| [`Control=`](/keys/control/) | `NORMAL` | How the samples are put together; see below. |
| [`Attack=`](/keys/attack/) | `1` with `ATTACK`, else `0` | How many leading samples are attack samples. |
| [`Decay=`](/keys/decay/) | `1` with `DECAY`, else `0` | How many trailing samples are decay samples. |

The flag values follow Yuri's Revenge, so its documentation of `Type=` and `Control=` applies. Flags are separated by spaces or commas; one the engine does not know is ignored.

`Type=` flags:

- `NORMAL` or `SCREEN`: the sound fades with the distance of its place from the edge of the view, vertical distance counting double, and is panned by where that place is across the view.
- `LOCAL`: as `SCREEN`, but measured from the centre of the view.
- `GLOBAL`: the fade stops at `MinVolume=`.
- `SHROUD` or `UNSHROUDED`: silent unless the cell of its place has been revealed.
- `SHROUDED`: silent unless the cell of its place is still unrevealed.
- `UNSHROUD`, `VIOLENT`, `MOVEMENT`, `QUIET`, `LOUD`, `PLAYER`, `NOISE_SHY`, `GUN_SHY` and `AMBIENT` are read and kept but decide nothing.

`Control=` flags:

- `NORMAL`: one sample, played once.
- `LOOP`: the body repeats, `Loop=` times or until ended.
- `RANDOM`: the body is one sample drawn at random, the same one for every cycle of a loop.
- `SEQUENTIAL`: the body is the next sample in turn each time the sound plays.
- `ALL`: the body is every body sample in order. `ALL` wins over `RANDOM`, which wins over `SEQUENTIAL`.
- `PREDELAY`: the `Delay=` silence comes before the first sample instead of between cycles.
- `INTERRUPT`: when `Limit=` is reached by a sound as loud as the new one, the oldest gives way instead of the new one being refused.
- `ATTACK`, `DECAY`: the list has attack or decay samples; `Attack=` and `Decay=` say how many.
- `QUEUE`: a sound refused for want of a voice or by `Limit=` waits up to two seconds for one instead.
- `AMBIENT` is read and kept but decides nothing.

## Files written for Tiberian Sun

The shipped `SOUND.INI` and `SOUND01.INI` hold `[SoundList]` and one section per sound with at most `Priority=`, and they read unchanged. An integer priority is kept as written, so the order in which those sounds give way to one another is what it was. Every other key takes its default, so such a sound is a one-shot at full volume that fades from the edge of the view over 28 cells, with three copies allowed at once. Sections copied from a Yuri's Revenge file read as well, with two differences: `Volume=1` is full volume rather than one percent, and a `Delay=` without a decimal point is in milliseconds.

Earlier OpenTS releases and the original game read `Volume=` as a multiplier and let a value above 1 make a sound louder than its sample. That value is now a percentage, so `Volume=2` is two percent. Remove such values or write `Volume=100`.
