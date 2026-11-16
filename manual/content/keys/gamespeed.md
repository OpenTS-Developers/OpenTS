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

The game controls dialog offers `0` through `6`, and confirming it saves the value to `sun.ini`. Outside a game the dialog shows this value. During a game it shows the speed the game plays at, or the player's pick while that order waits to take effect. A value of `7` set in `sun.ini` gives the same frame rate as `0`.

A campaign mission started from the main menu plays at this value. A skirmish plays at the speed chosen in its setup screen, and that choice replaces the value. A multiplayer game plays at the speed the host chooses in its setup screen and leaves the value alone. The skirmish setup screen and the host's multiplayer setup screen start at this value, or at `0` or `6` when the value is below `0` or above `6`.

A game started from [`SPAWN.INI`](/formats/spawn-ini/) plays at the speed that file sets and leaves the value alone. A resumed game against other machines is the exception: it starts at the speed it was saved with, and the lowest seat's file then sets the speed, as [Resuming a saved game](/formats/spawn-ini/#resuming-a-saved-game) describes.

A speed picked in the game controls dialog during a game is sent as an order. In a campaign mission or skirmish, it takes effect after the next frame and replaces the value, including in one started from `SPAWN.INI`. In a multiplayer game, it changes the speed for every player and leaves the value alone.

:::danger[Keep GameSpeed between 0 and 7]
The game does not range-check the value it reads from `sun.ini`. With `-1`, the game crashes with a division by zero the first time it stretches a delay of five frames or more, which a structure's animation or an EVA warning can do early in a mission. Any other value below `0` or above `7` reads outside the table the game uses to stretch delays shorter than five frames, so those delays take arbitrary lengths. A value below `-1` also turns delays of five frames or more into negative or zero lengths. The animations they time then advance every frame or stop, and repeated EVA warnings play with no pause.
:::
