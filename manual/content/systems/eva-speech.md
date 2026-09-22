---
title: EVA speech
summary: Speech lines wait in a queue of up to eight and play one at a time, after a one-second wait for the first line and half a second apart.
category: audio-speech
keys: [VoiceVolume]
---

Every line spoken during a game goes through one queue and plays one at a time. That includes EVA's announcements and the lines that [Play speech](/mapping/actions/taction-play-speech/) asks for. Up to eight lines can wait in the queue.

When a line is requested while nothing is speaking or waiting, the game waits about a second before playing it. The other lines an event requests over the next few frames join the queue during that second. After each line ends, the next one waits half a second.

A line whose file cannot be opened is skipped, and the next waiting line plays in its place.

A line can also be requested to play at once. It ends the one-second wait and goes ahead of the ordinary waiting lines, but it does not cut a line that is already speaking, and it still waits out the half-second gap. The incoming-transmission call that opens a radar movie is requested this way.

## Order

Waiting lines play in this order:

1. Mission accomplished and mission failed.
2. Lines requested to play at once.
3. Every other line, oldest first.

A line that is speaking or already waiting is not added again. When all eight places are taken, a new line replaces the oldest ordinary waiting line. If none is waiting, it replaces the oldest line requested at once. Mission accomplished and mission failed are never replaced.

## What stops a line

Stopping speech empties the queue and cuts the line that is speaking. It happens when:

- a mission is lost, once every waiting line has played or about five seconds pass;
- a scenario or saved game loads, because the game switches speech files.

Aborting a game stops speech at once. EVA then announces the exit, and speech stops again when that line ends or after about five seconds. While EVA is turned off, no exit announcement plays.

## Turning EVA off

An EVA line is one whose file name starts with `00-` or `01-`. [Disable Speech](/mapping/actions/taction-disable-speech/) keeps new EVA lines out of the queue until [Enable Speech](/mapping/actions/taction-enable-speech/) lets them in again. Other lines are still queued, and lines already waiting or speaking still play.

Each scenario starts with EVA enabled. A saved game restores the setting it had when it was saved.

## Volume

[`VoiceVolume`](/keys/voicevolume/) sets the volume of all speech. A change applies at once, including to the line that is speaking.

While any of these holds, every request to speak is refused and nothing is queued:

- the game was started with the [quiet launch option](/using/command-line/quiet/);
- `VoiceVolume` is below 1/255 (about 0.004), which includes zero;
- no audio device is available.
