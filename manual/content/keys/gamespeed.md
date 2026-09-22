---
key: GameSpeed
summary: The frame rate the game is held to, from 0 for the fastest to 6 for the slowest.
see_also: [ScrollRate, DetailLevel]
when_omitted:
  kind: value
  value: "3"
---

Each figure holds the game to a frame rate: 60 frames a second at `0`, 45 at `1`, 30 at `2`, 20 at `3`, 15 at `4`, 12 at `5` and 10 at `6`. The one exception is a single player mission or a skirmish at `0`, which is held to nothing and runs as fast as the machine manages. A network game runs at the lower of its figure's rate and the rate the slowest machine can sustain. A figure above `6` is held to 60.

The same figure rescales delays that have to keep their real-world timing whatever the frame rate is: building animations, infantry sequences, and the pauses between EVA reminders. Lowering the figure speeds the game up, but those delays do not shrink in proportion.

The in-game game controls dialog and the options screen both offer seven positions and write the choice back to `sun.ini`. A multiplayer lobby overwrites the figure with the speed the session settled on, and the in-game speed control issues an order that changes it for every player at once.

:::danger[A negative figure divides by zero and a large one reads past a table]
Nothing narrows the figure on the way in. `-1` makes the divisor zero, and the game then stops on a division fault the first time a delay of five frames or more is rescaled. A building animation or an EVA reminder does that within moments of a scenario starting. A figure of `8` or more reads past the end of the rescaling table used for the delays shorter than that. `7` is the last figure the table holds.
:::
