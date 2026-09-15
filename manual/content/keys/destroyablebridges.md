---
key: DestroyableBridges
summary: Whether a wall-destroying blast can bring a bridge span down.
see_also: [BridgeStrength, IonCannonWarhead, C4Warhead]
when_omitted:
  kind: value
  value: "yes"
  note: The special options are initialized with this built-in default when the game starts.
---

The switch is the first half of the test a blast makes when it reaches a span; the other half is [`Wall=yes`](/keys/wall/#scope-warheadtype) on the warhead. With the switch off, spans absorb everything and no bridge on the map can be brought down by fire. Once both halves pass, the span still comes down only when a chance roll against [`BridgeStrength`](/keys/bridgestrength/) favors the blast: a number picked between one and the figure lands below the blast's own damage. The ion cannon's warhead skips the roll. A trigger action that damages or destroys its target brings the span down regardless of the switch.

:::caution[The entry is read in campaigns only]
The `[SpecialFlags]` block is read from the map only in a single-player mission. Every other game type replaces the scenario's whole set of flags with the set the game options carry once the map has been read. Outside a campaign, bridge destruction follows the game options.
:::
