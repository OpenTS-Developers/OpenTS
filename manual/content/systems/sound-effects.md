---
title: Sound effects
summary: How a sound effect is put together from its SOUND.INI section, how it fades and pans with its place on screen, and how it competes for a voice.
category: audio-speech
keys: [Sounds, Priority, Volume, MinVolume, Range, Limit, Loop, Delay, FShift, VShift, Type, Control, Attack, Decay, Channels, SoundVolume]
---

Each sound effect is defined by its section in [SOUND.INI](/formats/sound-ini/). Every time the game plays the sound, that section decides which samples play, at what pitch and loudness, how far away the sound is heard, and whether it loops.

```ini title="SOUND.INI"
[MYGUN]
Sounds=MYGUN2 MYGUN3  ; body samples; RANDOM draws one for each play
Control=RANDOM INTERRUPT
Priority=NORMAL
Range=12              ; cells beyond the edge of the view where the sound falls silent
Limit=3               ; copies of MYGUN that may play at once
```

The section takes effect only for a sound ID registered under `[SoundList]`, as [SOUND.INI](/formats/sound-ini/) describes.

## Attack, body and decay

Each play has up to three parts: an attack sample, the body, and a decay sample. [`Control=`](/keys/control/) decides which [`Sounds=`](/keys/sounds/) samples form the body and how one is chosen.

Attack samples come from the start of the list and decay samples from its end, counted by [`Attack=`](/keys/attack/) and [`Decay=`](/keys/decay/). Without `Attack=`, `ATTACK` in `Control=` counts one sample, and a `Control=` line without `ATTACK` counts none. `Decay=` and `DECAY` work the same way. When several are listed, one of each is drawn at random. They are used only when the list has at least one sample left over for the body. Otherwise the whole list is body, and no attack or decay plays.

A play draws one pitch shift from [`FShift=`](/keys/fshift/) and one loudness change from [`VShift=`](/keys/vshift/). Both stay the same for every sample in that play, including every cycle of a loop.

## Loops and delays

With `LOOP` in `Control=`, the body repeats [`Loop=`](/keys/loop/) times, or with `Loop=0` until the game stops the sound. The attack plays once before the first cycle. Without [`Delay=`](/keys/delay/), the cycles follow one another with no gap.

With `Delay=`, each gap between cycles is a silence of random length from the `Delay=` range. `PREDELAY` in `Control=` adds one such silence before the first sample, whether or not the sound loops.

Each cycle after a `Delay=` gap competes for a voice again, as described under [Voices and priority](#voices-and-priority). A cycle that is refused ends the loop.

The decay plays after the last cycle, so a loop plays it only when `Loop=` is above 0. The game stops an endless loop by fading it out, and its decay sample does not play.

## Placed sounds

A sound played at a place in the world, such as an animation's sound, fades and pans with where that place is on screen. A sound played without a place, such as a button click, plays centered at the loudness the game asks for.

A placed sound is at full loudness anywhere inside the view. Outside the view, its loudness falls in a straight line to silence at [`Range=`](/keys/range/) cells beyond the edge. The distance used is the larger of the horizontal distance and twice the vertical distance, so that `Range=` counts cells in both directions. A sound that has faded below one twentieth of full is not started, so it falls silent slightly short of `Range=`.

The pan follows the place's horizontal position: centered in the middle of the view, and fully to one side at that edge of the view and beyond.

[`Type=`](/keys/type/) changes how the distance is judged:

- `LOCAL` measures the distance from the center of the view instead of its edge.
- `GLOBAL` stops the fade at [`MinVolume=`](/keys/minvolume/). A `MinVolume=` below `0.05` (5 percent) does not keep the sound audible, because anything quieter than one twentieth is cut off.
- `SHROUD` or `UNSHROUDED` silences the sound while its cell is unrevealed. `SHROUDED` silences it once its cell has been revealed.

While a placed sound plays, its loudness and pan follow the view as it scrolls. Once it fades below one twentieth of full, it stops and does not return when the view comes back.

An endless loop left at a waypoint by [Play Sound Effect At](/mapping/actions/taction-play-sound-at/) is the exception. It stays at its waypoint and stops while the waypoint is out of range. Whenever the waypoint is in range and the loop is not playing, it starts again without its attack sample.

These loops are kept in a [save game](/formats/save-games/) and resume after a load, also without their attack sample.

## Voices and priority

[`Limit=`](/keys/limit/) caps how many copies of one sound play at once. Copies still waiting to start, or silent between the cycles of a delayed loop, count toward it. At the limit, a new copy that is louder than the quietest copy replaces it. A new copy about as loud as the quietest replaces it only with `INTERRUPT` in `Control=`. Any other new copy is refused.

Up to [`Channels=`](/keys/channels/) sound effects play at once. Music, speech and movie sound do not count toward that number.

When every voice is busy, a new sound can take the voice of the playing sound with the lowest [`Priority=`](/keys/priority/). Among equals it takes the quietest, or the oldest when they are within a tenth of each other in loudness. It takes that voice when its priority is higher, or when the priorities are equal and the playing sound is more than a tenth quieter. Otherwise the new sound is refused.

A copy that replaces a playing copy under `Limit=` takes that copy's place, so it does not need to outrank anything. A copy that `Limit=` refuses never competes for a voice.

A refused sound does not play. If its `Control=` includes `QUEUE`, a sound refused when the game plays it keeps trying for up to two seconds before it is dropped. `QUEUE` does not cover a `PREDELAY` sound refused when its predelay ends, or a later cycle of a delayed loop.

## Loudness

A sound's level is its [`Volume=`](/keys/volume/) multiplied by its `VShift=` draw and by one more factor. For a placed sound, that factor is the distance fade. For a sound without a place, it is the loudness the game asks for. The result is capped at full loudness.

The [`SoundVolume`](/keys/soundvolume/) option then scales every sound effect, including those already playing.

While a movie from [Play Ingame Movie...](/mapping/actions/taction-play-ingame-movie/) plays in the radar, sound effects play at half level.

Levels follow the loudness curve of the original game's DirectSound playback, so a sound the game plays at half level sounds as it did there. On that curve, half level is about 10 dB quieter than full.

The mixer compresses peaks above nine tenths of full scale, so a loud mix does not clip harshly.

## Samples

A sound's samples are loaded when the sound first plays. Each name in `Sounds=` is looked up through the normal file search, which covers loose files and every mounted archive. The extensions are tried in order: `.WAV`, `.OGG`, `.FLAC`, `.MP3`, then `.AUD`, and the first file that decodes is used.

A sample must be mono or stereo, and no larger than eight megabytes once decoded to 16-bit audio. That is about 47 seconds of 44.1 kHz stereo. A larger sample does not load.

A sample that cannot be loaded is left out of the play. When the body is one chosen sample, the next body sample in the list plays in its place.

A decoded sample stays in memory while any play uses it. After that it stays until decoded samples take more than 64 megabytes, when the least recently used ones are dropped first.
