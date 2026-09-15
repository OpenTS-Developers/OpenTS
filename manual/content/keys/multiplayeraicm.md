---
key: MultiplayerAICM
summary: The percentage of its own starting money added to each computer house outside a campaign, one entry per difficulty.
see_also: ["system:difficulty"]
when_omitted:
  kind: value
  value: ""
  note: The list stays empty and the difficulty slot is used to index it anyway, reading storage that was never allocated.
---

The grant is worked out once, as a scenario outside a campaign finishes loading, for every computer house that is not [`MultiplayPassive=yes`](/keys/multiplaypassive/). The house's credits plus the value of the Tiberium it holds are multiplied by the entry as a percentage, and the result is handed to it **on top of** what it already had. An entry of `100` therefore leaves the house with twice its starting money, not the same amount, and an entry of `0` changes nothing.

Only the first three entries are ever read: entry 0 is the hardest game setting and entry 2 the easiest. Each computer house reads the entry at its own [difficulty slot](/systems/difficulty/#from-the-setting-to-a-slot), which outside a campaign is the setting chosen for the session, inverted. Once the session holds more than one human entry, the [easy-game bonus](/systems/difficulty/) moves that house one entry nearer entry 0, so a skirmish never takes that step. A campaign game never reaches this list.
