---
key: Harvester
scope: global-rules
label: IQ threshold
see_also: [IQ, HarvesterUnit, BuildRefinery]
when_omitted:
  kind: value
  value: "3"
---

A computer house whose [`IQ`](/keys/iq/) reaches this level replaces harvesters it has lost. The check runs at the head of the routine that picks the house's next vehicle, ahead of [the ordinary demand tally](/systems/ai-team-production/#production-demand). Choosing a harvester ends that pass: no other vehicle is selected until that harvester leaves the factory.

Three further conditions have to hold: the house must not be flagged as short of Tiberium, the harvester type must be within the house's tech level, and its refineries multiplied by a difficulty factor must outnumber its harvesters. That factor is `1` in a campaign game and for a house sitting in the `[Difficult]` slot, and `2` for every other house. A computer house therefore ordinarily keeps two harvesters per refinery, and only one where the factor is `1`.

Both counts read whole lists: every [`HarvesterUnit`](/keys/harvesterunit/) entry counts as a harvester here, and every [`BuildRefinery`](/keys/buildrefinery/) entry counts as a refinery. The replacement chosen is the first listed harvester type the house may own, falling back to the first listed type when it may own none.

:::caution[A harvester at `TechLevel=-1` is never replaced]
The tech level comparison is made between unsigned numbers, so `-1` on the harvester type reads as an enormous requirement rather than as no requirement, and the replacement is refused at every tech level.
:::

:::note[The difficulty slot is inverted for computer houses]
In a skirmish the `[Difficult]` slot is the one a computer house is given at the player's Easy setting, so that is where the one-harvester-per-refinery factor lands. [From the setting to a slot](/systems/difficulty/#from-the-setting-to-a-slot) covers the inversion, and [the computer's bonus with more than one human](/systems/difficulty/#the-computers-bonus-with-more-than-one-human) covers the further shift a session with several people applies on top of it.
:::
