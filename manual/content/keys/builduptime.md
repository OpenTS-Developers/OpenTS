---
key: BuildupTime
summary: The length of a structure's construction animation, in game minutes.
see_also: ["system:production"]
when_omitted:
  kind: value
  value: ".05"
---

The value is converted to frames and divided by the animation's step count when the buildup art is loaded, truncating once at the end; the result is the delay between one buildup frame and the next. The shipped `rules.ini` sets `.06`, which spreads 54 frames, or 3.6 seconds, across the type's steps; the engine default of `.05` spreads three seconds, and `1` spreads a full minute.

The step count is half the number of frames in the buildup art, or [`GateStages`](/keys/gatestages/) plus one for a [`Gate=yes`](/keys/gate/) type.

One value covers every structure in the game, so a type with more buildup frames spends less time on each of them rather than taking longer overall. [Buildup](/systems/production/#buildup) covers the game-speed adjustment applied to the resulting rate and the construction-yard handshake that runs alongside it.

A [`Theater=yes`](/keys/theater/) structure has its construction animation timed again as the theater is set up, from every frame in the file at a fixed five seconds, so this value does not reach it.
