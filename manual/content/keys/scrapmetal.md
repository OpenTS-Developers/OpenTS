---
key: ScrapMetal
summary: Whether wrecks leave the scrap animations their types name instead of their ordinary ones.
see_also: [ScrapExplosion, Explosion, "system:destruction-and-debris"]
when_omitted:
  kind: unchanged
  note: The scenario's special options are set up once when the game starts and are not reset between missions, so a campaign mission that omits the key keeps whatever the previously played mission established. Only the first mission of a session finds the initial value, no.
---

With the switch on, a destroyed object leaves the animations its type gives [`ScrapExplosion`](/keys/scrapexplosion/) rather than the ones it gives [`Explosion`](/keys/explosion/). A type that names no scrap animations is unaffected and explodes as it always does, so the switch can be thrown against a ruleset that has only converted part of its arsenal.

The switch reaches every kind of game, campaigns included, rather than being confined to a match against other players, and a saved game restores the one it was made under.

Because the animations are drawn with the shared random number generator, every machine in a match has to agree on this switch. A launch file settles it for all of them; a machine that disagrees will not stay in step.

:::caution[The entry is read in campaigns only]
The `[SpecialFlags]` block is read from the map only in a single-player mission or with the map debugger active, and every other game type replaces the scenario's whole set of flags with the one the lobby settled on once the map has been read. Outside a campaign, scrap wreckage follows the launch file's own option.
:::
