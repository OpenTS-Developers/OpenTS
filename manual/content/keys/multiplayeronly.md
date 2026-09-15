---
key: MultiplayerOnly
summary: Parsed flag that restricts nothing.
no_effect: true
when_omitted:
  kind: value
  value: "no"
---

The name promises a scenario that may be played only in a multiplayer game. The flag is stored from the map's `[Basic]` section and nothing reads it back. Neither the campaign progression nor the lobby's map list reads it, so no gameplay path reads it.
