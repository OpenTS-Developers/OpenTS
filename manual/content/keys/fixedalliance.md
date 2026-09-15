---
key: FixedAlliance
summary: Parsed flag that locks nothing.
no_effect: true
when_omitted:
  kind: value
  value: "no"
  note: The special options are initialized with this built-in default when the game starts.
---

Two conditions have to meet for this flag to matter, and they exclude each other. The map's `[SpecialFlags]` block is parsed only in a single-player mission, while the one routine that reads the locked-alliance state refuses to run there: it is the command that offers to ally with the owner of a selected object. Every game type that could reach the command replaces the scenario's flags wholesale as soon as the map has been read. Nothing the map wrote survives to be tested.

Locked alliances come from the internet setup instead. It sets the state directly when the alliances were dictated from outside the game, and clears it when the players are free to negotiate. Whatever the map says here reaches no gameplay path at all.
