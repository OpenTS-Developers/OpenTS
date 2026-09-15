---
key: GameForming
summary: Sound played in the network game list when a joinable game appears.
see_also: [GameClosed, PlayerJoined, PlayerLeft, SystemError]
when_omitted:
  kind: value
  value: none
---

```ini title="rules.ini"
[AudioVisual]
GameForming=NEWGAME1 ; a sound ID registered in SOUND.INI
```

The sound belongs to the network game list a player sits in before joining a LAN match, and it is played without a position. Two announcements use it: a game already on the list reopening after having been closed, and a game not seen before arriving already open. A game that appears closed is added to the list in silence.

Both announcements are recognized from the answers hosts broadcast about themselves, and both are skipped once the player has been confirmed into a game. From that point the list announces players rather than games. The chat panel prints a matching line beside each sound.
