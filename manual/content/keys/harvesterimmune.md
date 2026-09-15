---
key: HarvesterImmune
summary: Exempts harvesters from being shot at and from taking blast damage.
see_also: [HarvesterUnit, LimpetFactor, "system:target-selection"]
when_omitted:
  kind: value
  value: "no"
  note: The special options are initialized with this built-in default when the game starts.
---

```ini title="map file"
[SpecialFlags]
HarvesterImmune=yes
```

Four separate protections switch on together. A harvester is dropped from the list of objects an explosion collects, so it is never caught in a blast aimed at something beside it. Damage aimed at the harvester itself is refused whenever it arrives with a warhead whose [`LimpetFactor`](/keys/limpetfactor/) is not positive, so limpet drones remain able to attach, and damage delivered without a warhead at all still lands. [Target evaluation](/systems/target-selection/) rejects a harvester as a candidate, so nothing picks one on its own. A vehicle thief ordered onto an enemy harvester selects it rather than moving to capture it. Two last effects wait for a harvester's death: its Tiberium cargo is not scattered over the ground beside it, and the [`[CombatDamage]` TiberiumExplosive entry](/keys/tiberiumexplosive/#scope-global-rules) does not detonate it.

:::caution[Two different definitions of "harvester" are in play]
All four test membership of the [`HarvesterUnit`](/keys/harvesterunit/) list. One further protection does not: the hunter-seeker drone, which chooses at random among every enemy object on the map, passes over any unit whose type harvests, listed or not.
:::

:::caution[The entry is read in campaigns only]
The `[SpecialFlags]` block is read from the map only in a single-player mission. Every other game type replaces the scenario's whole set of flags with the set the game options carry once the map has been read. Outside a campaign, a match against other machines takes the truce from the game options, and a skirmish never applies it at all. While it is on outside a campaign, a defeated player's vehicles are destroyed and every type named in [`HarvesterUnit`](/keys/harvesterunit/) is discounted when deciding that a player has been defeated.
:::
