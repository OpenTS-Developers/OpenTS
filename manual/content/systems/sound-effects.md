---
title: Sound effects
summary: A sound effect plays as an event built from its SOUND.INI section, is placed in the view by where its source is on screen, and competes for one of sixteen voices by priority.
category: audio-speech
keys: [Sounds, Priority, Volume, MinVolume, Range, Limit, Loop, Delay, FShift, VShift, Type, Control, Attack, Decay, Channels, SoundVolume]
---

Every sound effect the game plays goes through its [SOUND.INI](/formats/sound-ini/) section. Playing it builds one event: an attack sample, the body, and a decay sample, chosen as the section's [`Control=`](/keys/control/) says, with a pitch and loudness drawn once from [`FShift=`](/keys/fshift/) and [`VShift=`](/keys/vshift/). The mixer plays the whole sequence on one voice without a gap between samples, and a loop repeats its body until the game ends the event or [`Loop=`](/keys/loop/) cycles have played.

## Placed sounds

A sound played at a place in the world is heard from where that place is on screen. Its loudness falls in a straight line from full at the edge of the view to silence at [`Range=`](/keys/range/) cells beyond it, vertical distance counting double, and it is panned by where the place is across the view. The engine re-aims every placed sound each tick, so scrolling away quietens it and scrolling back brings it up again. [`Type=`](/keys/type/) moves the measure to the centre of the view, gives the fade a floor, or ties the sound to whether its cell has been revealed. A sound played without a place, as a button click is, is not attenuated or panned.

Sounds left at a waypoint by a trigger and sounds attached to objects are kept in tables of their own and re-aimed the same way; a looping one that scrolls out of range stops and starts again without its attack when its place comes back. Both tables travel with a [save game](/formats/save-games/).

## Voices and priority

Up to [`Channels=`](/keys/channels/) sound effects play at once, sixteen unless the file says otherwise; music, speech and movie sound have voices of their own. A sound that would exceed its section's [`Limit=`](/keys/limit/) is refused or takes the place of the quietest copy playing, and when every voice is taken a new sound displaces the playing sound with the lowest [`Priority=`](/keys/priority/) only when it outranks it. A refused sound whose section carries `QUEUE` waits up to two seconds for a voice.

## Loudness

The level a sound plays at is the product of the loudness the game asked for, the section's [`Volume=`](/keys/volume/), the distance fade and the random draw, put through the same loudness curve the original DirectSound path used, so a sound at half level is heard as it was. The [`SoundVolume`](/keys/soundvolume/) setting is the level of the whole sound effect group and applies to sounds already playing. When the mix would clip, the loudest peaks are softened rather than cut.

## Samples

A section's samples are looked up when the sound first plays, through the file layer that sees loose files and every mounted archive, as `.WAV`, `.OGG`, `.FLAC`, `.MP3` and then `.AUD`. A decoded sample stays in memory while any event uses it and afterwards until the memory is wanted for another, within a budget of sixty-four megabytes.
