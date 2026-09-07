---
key: HarvesterImmune
summary: Exempts harvesters from being shot at and from taking blast damage.
see_also: [HarvesterUnit, LimpetFactor, "system:target-selection"]
when_omitted:
  kind: unchanged
  note: The scenario's special options are set up once when the game starts and are not reset between missions, so a campaign mission that omits the key keeps whatever the previously played mission established. Only the first mission of a session finds the initial value, no.
---

```ini title="map file"
[SpecialFlags]
HarvesterImmune=yes
```

Four separate protections switch on together. A harvester is dropped from the list of objects an explosion collects, so it is never caught in a blast aimed at something beside it. Damage aimed at the harvester itself is refused whenever it arrives with a warhead whose [`LimpetFactor`](/keys/limpetfactor/) is not positive, so limpet drones remain able to attach, and damage delivered without a warhead at all still lands. [Target evaluation](/systems/target-selection/) rejects a harvester as a candidate, so nothing picks one on its own. A vehicle thief ordered onto an enemy harvester selects it rather than moving to capture it.

:::caution[Two different definitions of "harvester" are in play]
All four test membership of the [`HarvesterUnit`](/keys/harvesterunit/) list. One further protection does not: the hunter-seeker drone, which chooses at random among every enemy object on the map, passes over any unit whose type harvests, listed or not.
:::

:::caution[The entry is read in campaigns only]
The `[SpecialFlags]` block is read from the map only in a single-player mission or with the map debugger active, and every other game type replaces the scenario's whole set of flags with the one the lobby settled on once the map has been read. Outside a campaign the harvester truce comes from the lobby option, which also destroys every vehicle a defeated player owned and discounts every type named in [`HarvesterUnit`](/keys/harvesterunit/) when deciding that a player has been defeated.
:::
