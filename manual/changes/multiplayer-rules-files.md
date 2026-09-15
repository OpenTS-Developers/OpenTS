---
title: Read multiplayer rules from MPLAYER.INI
category: feature
release: 0.2.0
targets:
- type: format
  id: multiplayer-rules
  effect: added
- type: format
  id: opents-ini
  effect: changed
- type: format
  id: rules-registries
  effect: changed
credit:
- ZivDero
---

Every game that is not a campaign, skirmish included, now reads
[`MPLAYER.INI`](/formats/multiplayer-rules/) over the rest of the rules, and
`MPLAYERFS.INI` as well while Firestorm is enabled. Both files are optional and
accept everything `RULES.INI` accepts.

They are layered after the expansion and translated rules and before the
scenario, so a map still overrides them. [`OPENTS.INI`](/formats/opents-ini/)
names them with `MultiplayerRules=` and `MultiplayerRulesExpansion=`.

Both count toward the rules checksum a host compares against a joining player,
so every machine in a game needs the same copy. A deployment that ships neither
file produces the same checksum it did before.
