---
title: Music
summary: Music tracks play one at a time, fade out over a second and a half when another is queued, and pause with all other game sound while the game is in the background.
category: audio-speech
keys: [ScoreVolume, IsScoreRepeat, IsScoreShuffle]
---

The game plays one music track at a time. [THEME.INI](/formats/theme-ini/) declares the tracks, and each track streams from the `.AUD` file named after its ID. [AUD audio](/formats/aud/) explains where the game looks for that file.

The player's music settings are stored in `sun.ini` under `[Audio]`, and the sound options screen changes them. [`ScoreVolume`](/keys/scorevolume/) sets the music volume. [`IsScoreShuffle`](/keys/isscoreshuffle/) picks the next track at random, and [`IsScoreRepeat`](/keys/isscorerepeat/) plays every track again when it ends. The values below are examples; the key pages give the defaults.

```ini title="sun.ini"
[Audio]
ScoreVolume=1.0
IsScoreShuffle=yes
IsScoreRepeat=no
```

## Changing tracks

A queued track fades out the current track over a second and a half, then starts once the fade has finished. The [Play music theme](/mapping/actions/taction-play-music/) trigger action and the [Play music](/mapping/missions/tmission-play-music/) team mission queue their track.

Only one track can wait at a time, so a request to queue a track is ignored while another track is waiting.

The main menu, map selection, the score screen and ion storms start their tracks immediately instead. The current track stops at once, with no fade.

On the sound options screen, playing a track from the list stops the current track at once and starts the chosen one.

The Stop button on the sound options screen fades the current track out. No music plays again until a track starts immediately, the player plays a track from the list, or a new scenario starts. Play music theme actions and Play music team missions are ignored in the meantime.

Starting a scenario stops the current track. The game then plays the scenario's [`Theme=`](/keys/theme/) track if it names one, and otherwise the next allowed track. There are two exceptions:

1. In a campaign mission with no briefing movie and no action movie, the `Theme=` track starts at once behind the mission briefing screen. When the mission begins, that track fades out and the next allowed track follows, unless the `Theme=` track repeats.
2. In a scenario with an action movie, the game requests the `Theme=` track only when that movie plays. If the movie does not play, for example when the player restarts a mission after a defeat, the game starts with the next allowed track.

A request for a track whose file cannot be played produces no music. A queued request still fades out the current track, and an immediate one still stops it. During a game, the next allowed track then starts.

## Choosing the next track

When a track ends, the game chooses the next one from the allowed tracks. With shuffle on, it picks at random and avoids the track that just ended. With shuffle off, it takes the next allowed track in THEME.INI order and wraps around at the end of the list.

A track is allowed when all of these hold:

1. The game found its `.AUD` file.
2. It has [`Normal=yes`](/keys/normal/).
3. Its [`Side=`](/keys/side/#scope-themes) is unset or matches the side of the player's country.
4. In a campaign mission, the current mission number has reached its [`Scenario=`](/keys/scenario/#scope-themes).

A track with [`Repeat=yes`](/keys/repeat/) plays again each time it ends, whatever the shuffle setting. The repeat option does the same for every track.

A repeating track that the game chose, or that was queued, gives way to the next queued request. A repeating track that was started immediately ignores queued requests, so Play music theme actions and Play music team missions have no effect while it plays. It keeps playing until another track starts immediately, the player uses the Play or Stop button on the sound options screen, or a new scenario starts. With the repeat option on, this lasts for the rest of the mission after an ion storm.

A track that was started immediately does not lead into the next allowed track. When it ends, the music stops unless the track repeats or another track has been queued. In a game, examples are the ion storm track, the track that restarts from the beginning when the storm ends, and the track that restarts after the player replays the briefing video during a mission.

## Focus and volume

While the player has switched to another application, all game sound pauses where it is, music included. It resumes from the same point when the player switches back.

A change to the music volume applies at once to the track already playing.

At zero, no track starts, and a request to queue a track is discarded. A track that was already playing continues without sound. When the volume is raised again, that track becomes audible from the point it has reached. If it has ended in the meantime, the next track is chosen as described above. A track started immediately while the volume is zero stops the silent track and waits. It plays from the beginning when the volume is raised.
