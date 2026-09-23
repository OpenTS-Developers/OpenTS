---
key: ScrapMetal
summary: Whether wrecks leave the scrap animations their types name instead of their ordinary ones.
see_also: [ScrapExplosion, Explosion, "system:destruction-and-debris"]
when_omitted:
  kind: value
  value: "no"
  note: The game's scrap option is copied into the flags before every scenario read, so a mission that omits the key plays under it. The option is off unless a launch file for a skirmish or network game turns it on, and a campaign map's own assignment may overrule it.
---

With the switch on, a destroyed object leaves the animations its type gives [`ScrapExplosion`](/keys/scrapexplosion/) rather than the ones it gives [`Explosion`](/keys/explosion/). A type that names no scrap animations is unaffected and explodes as it always does, so the switch can be thrown against a ruleset that has only converted part of its arsenal.

The switch reaches every kind of game, campaigns included, rather than being confined to a match against other players. A saved game restores the one it was made under.

Because the animations are drawn with the shared random number generator, every machine in a match has to agree on this switch.

A [launch file](/formats/spawn-ini/#the-options-every-house-plays-under) settles it for a skirmish or a game against other machines, and a machine that disagrees will not stay in step. A campaign launched from a file does not take it from the file.

:::caution[The entry is read in campaigns only]
The `[SpecialFlags]` block is read from the map only in a single-player mission. Every other game type replaces the scenario's whole set of flags with the set the game options carry once the map has been read. Outside a campaign, scrap wreckage follows the launch file's own option.
:::
