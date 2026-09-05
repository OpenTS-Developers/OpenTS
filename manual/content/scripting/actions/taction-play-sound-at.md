---
type: action
id: TACTION_PLAY_SOUND_AT
title: Play Sound Effect At...
summary: Plays a sound effect at the trigger's waypoint, and keeps a looping one there.
caveats:
  - The sound is positioned on the map, so it fades with the distance from the waypoint to the view and is inaudible once the waypoint is far enough off screen. Its section in SOUND.INI sets how far that is.
  - A one-shot sound that is out of range when the action fires does not play later; the action has to fire again.
related:
  - type: action
    id: TACTION_PLAY_SOUND
  - type: action
    id: TACTION_PLAY_SOUND_RANDOM
---

## A looping sound stays put

A sound whose section carries `Control=LOOP` with no loop count is left at the waypoint after the action fires. It follows the view: it quietens as the view scrolls away, stops out of range, and starts again, without its attack sample, when the waypoint comes back into view. Up to two hundred such sounds are kept at once, and they travel with a [save game](/formats/save-games/) and resume on load. The scenario's end clears them.

A sound with any other section plays once, attenuated by where the waypoint is, exactly as [Play Sound Effect](/mapping/actions/taction-play-sound/) would play it if it were positioned.
