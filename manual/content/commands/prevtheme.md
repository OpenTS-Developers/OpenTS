---
command_id: PrevTheme
---

Plays the nearest allowed music track before the one playing and shows its name on screen for about a second. The track list wraps from the first track to the last. With no track playing, the command starts at the last allowed track. [Music](/systems/music/#choosing-the-next-track) lists what makes a track allowed. While [`NowPlaying=yes`](/keys/nowplaying/) and a scenario is under way, this message is not posted. The bottom-left line names the track once it is playing.

The current track stops at once, with no fade. If no track is allowed, nothing happens and the current track keeps playing. If the track playing is the only allowed track, it starts again from the beginning.

With the music volume at zero, the command stops the current track and still shows the chosen track's name, but that track does not start. During a scenario with `NowPlaying=yes`, that message is not posted, and the bottom-left line leaves because the track does not start.
