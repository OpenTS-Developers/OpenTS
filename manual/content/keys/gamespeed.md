---
key: GameSpeed
summary: The frame rate the game is held to, from 0 for the fastest to 6 for the slowest.
see_also: [ScrollRate, DetailLevel]
when_omitted:
  kind: value
  value: "3"
---

Each value holds the game to a frame rate, in frames per second. A campaign mission or skirmish uses a faster table than a multiplayer game at every value except `6`:

| Value | `0` | `1` | `2` | `3` | `4` | `5` | `6` |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Multiplayer game | 60 | 45 | 30 | 20 | 15 | 12 | 10 |
| Campaign mission or skirmish | No limit | 60 | 45 | 30 | 20 | 15 | 10 |

With no limit, the game runs as fast as the computer allows. A multiplayer game runs at its value's rate or at the rate the slowest player's computer can sustain, whichever is lower.

The value also stretches some delays, more at faster speeds, so that they speed up less than the rest of the game. These are animations whose art sets [`Normalized=yes`](/keys/normalized/), a structure's construction and idle animations, infantry idle animations, and the pause between repeated EVA warnings that [`SpeakDelay`](/keys/speakdelay/) sets.

The game controls dialog offers `0` through `6` and saves the chosen value to `sun.ini`. A value of `7` set in `sun.ini` gives the same frame rate as `0`. A change from the dialog or the speed commands keeps the value in the range `0` to `6`. The skirmish and multiplayer setup screens replace the value with the speed chosen there, and a game started from [`SPAWN.INI`](/formats/spawn-ini/) uses the speed that file names. During a multiplayer game, a speed change from the game controls dialog is sent as an order that changes the speed for every player. The [Increase Game Speed](/commands/increasegamespeed/) and [Decrease Game Speed](/commands/decreasegamespeed/) commands also change the speed through hotkeys. Those commands are not available in a multiplayer game.

:::danger[Keep GameSpeed between 0 and 7]
The game does not range-check the value it reads from `sun.ini`. With `-1`, the game crashes with a division by zero the first time it stretches a delay of five frames or more, which a structure's animation or an EVA warning can do early in a mission. Any other value below `0` or above `7` reads outside the table the game uses to stretch delays shorter than five frames, so those delays take arbitrary lengths. A value below `-1` also turns delays of five frames or more into negative or zero lengths. The animations they time then advance every frame or stop, and repeated EVA warnings play with no pause.
:::
