---
key: HarvesterImmune
summary: Exempts harvesters from being shot at and from taking blast damage.
see_also: [HarvesterUnit, LimpetFactor, "system:target-selection"]
when_omitted:
  kind: value
  value: "no"
  note: Each scenario starts with this built-in default.
---

```ini title="map file"
[SpecialFlags]
HarvesterImmune=yes
```

`HarvesterImmune=yes` declares a harvester truce. While it is on, a vehicle whose type is listed in [`HarvesterUnit`](/keys/harvesterunit/) is protected in four ways:

- Explosions never damage it, including a blast aimed at something beside it.
- Damage that carries a warhead is refused, unless the warhead's [`LimpetFactor`](/keys/limpetfactor/) is above `0`.
- [Target selection](/systems/target-selection/) rejects it, so no object picks it as a target on its own.
- A vehicle thief cannot be ordered to steal it: clicking an enemy harvester with the thief selects the harvester instead.

Two further protections cover every destroyed vehicle that carries Tiberium, listed in `HarvesterUnit` or not. Its load is not spilled onto the ground around it, and [`TiberiumExplosive`](/keys/tiberiumexplosive/#scope-global-rules) in `[CombatDamage]` does not make it explode.

While the truce is on, the hunter-seeker drone skips any vehicle whose type is [`Harvester=yes`](/keys/harvester/#scope-unittype), whether or not it is listed in `HarvesterUnit`.

Outside single-player missions the truce has two more effects. When a player is defeated, all of that player's vehicles are taken off the map. Outside a short game, vehicles listed in `HarvesterUnit` do not count toward keeping a player in the game.

:::caution[The entry is read in campaigns only]
Only a single-player mission reads `[SpecialFlags]` from the map. In a skirmish or a game against other machines, the game's harvester truce option sets the truce: the launch file's `HarvesterTruce`, or the truce checkbox in the network lobby. The skirmish setup screen has no truce option, so a skirmish set up from the menu has the truce off.
:::
