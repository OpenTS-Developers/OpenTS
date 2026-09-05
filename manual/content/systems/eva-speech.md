---
title: EVA speech
summary: EVA lines are queued and spoken one at a time through the speech stream, with a one second settle before a burst and half a second between lines.
category: audio-speech
keys: [VoiceVolume]
---

Every request to speak a line goes into a queue that holds up to eight lines. Nothing is spoken for a second after the first request of a burst, so the lines that a single event scatters across a few frames collect before the first is heard, and half a second of silence separates one line from the next. A line asked for at once, as the incoming-transmission call of a radar movie is, skips the settle and goes ahead of the queue, but never cuts a line that is already speaking.

## Order

Lines wait by class, then by priority, then by age. Mission accomplished and mission failed are critical and go ahead of everything waiting; every other line is queued at one priority, so the queue plays them in the order they were asked for. A line that is speaking or already waiting is not queued a second time. When the queue is full, the oldest of the lowest-priority lines makes room, and a critical line gives way only to another.

## What stops a line

Stopping speech empties the queue and cuts the line that is speaking. The scenario's end and the switch of speech files at the start of a scenario both do this, so a line never plays past the archive it came from. Turning EVA off through the trigger system keeps her own lines out of the queue while letting the other speech through; the state travels with a save game.

## Volume

The voice setting scales the whole speech stream and takes effect on the line that is speaking. Speech is silent when the setting is zero, when the game was started quiet, or when there is no audio device, and nothing is queued in that state either.
