---
key: GameSpeed
summary: The pace the game is held to, counted in sixtieths of a second between frames.
see_also: [ScrollRate, DetailLevel]
when_omitted:
  kind: value
  value: "3"
---

A frame is not begun until the delay has run out, so a larger figure gives a slower game. `0` runs as fast as the machine manages, and `3` holds the game to twenty frames a second at most. A single player mission, a skirmish, and a network game still using the older command protocol all run on this delay. A network game on the current protocol turns the figure into a frame rate instead: 60 at `0`, 45 at `1`, and sixty divided by the figure above that. Whichever is lower wins, that frame rate or the rate the machines can sustain.

The same figure rescales delays that have to keep their real-world timing whatever the frame rate is: building animations, infantry sequences, and the pauses between EVA reminders. Lowering the figure speeds the game up, but those delays do not shrink in proportion.

The in-game game controls dialog and the options screen both offer seven positions and write the choice back to `sun.ini`. A multiplayer lobby overwrites the figure with the speed the session settled on, and the in-game speed control issues an order that changes it for every player at once.

:::danger[A negative figure divides by zero and a large one reads past a table]
Nothing narrows the figure on the way in. `-1` makes the divisor zero, and the game then stops on a division fault the first time a delay of five frames or more is rescaled. A building animation or an EVA reminder does that within moments of a scenario starting. A figure of `8` or more reads past the end of the rescaling table used for the delays shorter than that. `7` is the last figure the table holds.
:::
