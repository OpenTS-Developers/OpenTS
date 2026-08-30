---
key: PlayerJoined
summary: Sound played in a network lobby when another player enters the game being set up.
see_also: [PlayerLeft, GameForming, GameClosed, OptionsChanged]
when_omitted:
  kind: value
  value: none
---

```ini title="rules.ini"
[AudioVisual]
PlayerJoined=PLYRJOIN ; a sound ID registered in SOUND.INI
```

The sound belongs to the LAN lobby and is played without a position. In the LAN lobby two announcements use it: a guest hears it as a new player is added to the player list, but only once the guest's own join has been confirmed, and the host hears it as it accepts a join query and sends its confirmation back. Player entries seen while the guest is still browsing the game list are therefore added silently; confirmation clears the list and asks again, so the players already present are then announced in a burst. Both announcements mark a player entering the game rather than the chat room — a player who is only in the chat list makes no sound.
