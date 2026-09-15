---
key: Players
summary: The most players a multiplayer game will seat.
when_omitted:
  kind: value
  value: "8"
---

The figure caps a multiplayer session. A player trying to join a game that already holds this many is turned away with the message that the game is full. In a network game, the keys from F1 up open the in-game message line, but only this many of them respond. The last of the run addresses everyone and the ones before it address the individual players in connection order. A key past the players actually connected does nothing.

```ini title="rules.ini"
[Maximums]
Players=8
```

The read is guarded by the section: without a `[Maximums]` section the figure is never looked for. A section with no `Players` and no section at all come to the same thing. Stock rules give `8`, which is the figure the engine starts from as well. The assignment sizes nothing despite the section it sits in: the houses come from the scenario and the players from the session, and a larger figure creates neither.
