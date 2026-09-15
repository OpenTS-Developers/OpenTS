---
title: Sound effects
summary: A sound effect plays as an event built from its SOUND.INI section, is placed in the view by where its source is on screen, and competes for one of sixteen voices by priority.
category: audio-speech
keys: [Sounds, Priority, Volume, MinVolume, Range, Limit, Loop, Delay, FShift, VShift, Type, Control, Attack, Decay, Channels, SoundVolume]
---

Every sound effect the game plays goes through its [SOUND.INI](/formats/sound-ini/) section. Playing it builds one event: an attack sample, the body, and a decay sample, chosen as the section's [`Control=`](/keys/control/) says. A pitch is drawn once from [`FShift=`](/keys/fshift/), and a loudness from [`VShift=`](/keys/vshift/). The mixer plays the whole sequence on one voice, without a gap between samples. A loop repeats its body until the game ends the event, or until [`Loop=`](/keys/loop/) cycles have played.

```ini title="SOUND.INI"
[MYGUN]
Sounds=MYGUN2 MYGUN3  ; body samples, one of them drawn for each play
Control=RANDOM INTERRUPT
Priority=NORMAL
Range=12              ; cells past the edge of the view before it fades out
Limit=3
```

## Placed sounds

A sound played at a place in the world is heard from where that place is on screen. Its loudness falls in a straight line from full at the edge of the view to silence at [`Range=`](/keys/range/) cells beyond it, and vertical distance counts double. It is panned by where the place is across the view. The engine re-aims every placed sound each tick, so scrolling away quietens it and scrolling back brings it up again. [`Type=`](/keys/type/) moves the measure to the center of the view, gives the fade a floor with [`MinVolume=`](/keys/minvolume/), or ties the sound to whether its cell has been revealed. A sound played without a place, as a button click is, is not attenuated or panned.

Sounds left at a waypoint by a trigger and sounds attached to objects are kept in tables of their own, and both are re-aimed the same way. A looping one that scrolls out of range stops, then starts again without its attack when its place comes back. Both tables travel with a [save game](/formats/save-games/).

## Voices and priority

Up to [`Channels=`](/keys/channels/) sound effects play at once, sixteen unless the file says otherwise. Music, speech and movie sound are not counted against that number. A sound that would exceed its section's [`Limit=`](/keys/limit/) is refused, or else it takes the place of the quietest copy already playing. When every voice is taken, a new sound displaces the playing sound with the lowest [`Priority=`](/keys/priority/), but only when it outranks that sound. A refused sound whose section's [`Control=`](/keys/control/) is `QUEUE` waits up to two seconds for a voice.

## Loudness

The level a sound plays at is the product of four things: the loudness the game asked for, the section's [`Volume=`](/keys/volume/), the distance fade, and the random draw. That product goes through the same loudness curve the original DirectSound path used, so a sound at half level is heard as it was. The [`SoundVolume`](/keys/soundvolume/) setting is the level of the whole sound effect group, and it applies to sounds already playing. When the mix would clip, the loudest peaks are softened rather than cut.

## Samples

A section's samples are looked up when the sound first plays, through the file layer that sees loose files and every mounted archive. The extensions are tried in order: `.WAV`, `.OGG`, `.FLAC`, `.MP3`, then `.AUD`. A decoded sample stays in memory while any event uses it and afterwards until the memory is wanted for another, within a budget of sixty-four megabytes.
