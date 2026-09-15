---
title: EVA speech
summary: EVA lines are queued and spoken one at a time through the speech stream, with a one second pause before a burst and half a second between lines.
category: audio-speech
keys: [VoiceVolume]
---

Every request to speak a line goes into a queue that holds up to eight lines. Nothing is spoken for a second after the first request of a burst, so the lines a single event scatters across a few frames collect before the first is heard. Half a second of silence then separates one line from the next. A line asked for at once skips that pause and goes ahead of the queue, but never cuts a line that is already speaking. The incoming-transmission call of a radar movie is asked for that way.

## Order

Lines wait by class, then by priority, then by age. Mission accomplished and mission failed are critical and go ahead of everything waiting. Every other line is queued at one priority, so the queue plays them in the order they were asked for. A line that is speaking or already waiting is not queued a second time. When the queue is full, the oldest of the lowest-priority lines in the lowest class makes room, and a critical line gives way only to another.

## What stops a line

Stopping speech empties the queue and cuts the line that is speaking. The scenario's end does this, and so does the switch of speech files at the start of a scenario. A line never plays past the archive it came from. Turning EVA off through the trigger system keeps her own lines out of the queue while letting other speech through. [Disable Speech](/mapping/actions/taction-disable-speech/) is the action that does it and [Enable Speech](/mapping/actions/taction-enable-speech/) is the one that undoes it. Whether EVA may speak travels with a save game.

## Volume

The `VoiceVolume` setting scales the whole speech stream and takes effect on the line that is speaking. Speech is silent while any of these hold, tested in this order:

- the game was started with the quiet launch option;
- `VoiceVolume` is zero;
- no audio device is available.

Nothing is queued in that state either.
