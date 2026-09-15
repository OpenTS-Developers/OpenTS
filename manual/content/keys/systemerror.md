---
key: SystemError
summary: Sound played when the LAN game list refuses a join attempt.
see_also: [GameForming, GameClosed, PlayerJoined, PlayerLeft]
when_omitted:
  kind: value
  value: none
---

```ini title="rules.ini"
[AudioVisual]
SystemError=BUZZER1 ; a sound ID registered in SOUND.INI
```

The sound belongs to the LAN game list and is played without a position, always beside a system line printed into the chat panel explaining the refusal. Seven refusals are checked before a join request is even sent:

- no game selected;
- no game left in the list to join;
- an empty player handle;
- a game that is no longer open;
- a Firestorm game with the expansion not installed;
- a Firestorm game with the expansion installed but not enabled;
- a base-game host approached from a Firestorm client.

An eighth plays it when a request that was sent comes back rejected by the host because the name is already taken, because the game is full, or for any other reason the host gives.

Nothing outside that list uses it, so it is not a general-purpose error sound: an interface refusal during a match takes [`ScoldSound`](/keys/scoldsound/) instead.
