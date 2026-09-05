---
key: Brief
summary: The mission briefing movie, replayable from the objectives screen.
see_also: [Intro, Action, Win, Lose, PostScore, PreMapSelect]
when_omitted:
  kind: value
  value: "<none>"
---

```ini title="map file"
[Basic]
Brief=GDI_M02
```

The movie runs directly after [`Intro`](/keys/intro/) when a campaign mission is started fresh, and it is the only one of the mission's movies that stays reachable afterwards: the objectives screen shows a second button beside "resume" when a briefing movie is set, and that button stops the score, replays the movie, and starts the score again. With no briefing movie, the resume button is centered on its own and the screen offers text alone.

A mission that names no briefing movie, or names one whose file is missing, shows that same objectives screen as it starts, in the movie's place. It appears only on a fresh start: a restart, and the replay offered after a loss, go straight to the map. The mission's transit [`Theme`](/keys/theme/) plays behind it.

Naming a movie the art file's `[Movies]` list does not carry has the same effect as omitting the key. [`Intro`](/keys/intro/) covers how a movie name is resolved and what happens to one that cannot be found.

The briefing movie is also formatted into a 25-byte buffer on the way past, with no length check and no use made of the result. A `[Movies]` entry longer than twenty characters overruns that buffer on the stack.
