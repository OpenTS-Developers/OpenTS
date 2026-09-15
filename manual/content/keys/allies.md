---
key: Allies
summary: The houses a scenario's house counts as friendly.
see_also: [Owner, AllyReveal, Edge]
when_omitted:
  kind: value
  value: ""
  note: An empty list, which leaves the house allied to nobody but itself.
---

The value is a comma-separated list of house identifiers. Each name is matched without regard to letter case against both the section names and the display names of the loaded houses, and a name that matches nothing contributes nothing. A campaign mission reads it from each house record. A skirmish or multiplayer game reads it instead from the [spawn house](/formats/scenario-objects/#spawn-houses) sections `[Spawn1]` through `[Spawn8]`, and each section belongs to the house starting at that position. There a name may also be a spawn house: the house allies with the one house holding that position. A country name allies it with every house playing that country. A section for a position nobody holds is ignored, and an observer is never allied.

```ini title="scenario map file"
[Special] ; a house record in the scenario's own house list
Allies=GDI,Nod
```

```ini title="multiplayer map file"
[Spawn1] ; whoever starts at waypoint 0
Allies=Spawn2,Nod
```

An alliance is what keeps two houses from turning on each other: an allied object is [rejected by an automatic target scan](/systems/target-selection/#why-a-candidate-is-rejected), is passed over rather than crushed, and does not provoke [retaliation](/systems/target-selection/#retaliation). A house is always its own ally whether or not it names itself.

:::caution[The alliance runs one way]
Naming a house here makes *this* house treat that one as a friend. It does not make that house treat this one as a friend, and a scenario that wants a mutual truce has to write the pairing into both house records. The check that decides whether every house left alive is allied asks the question in both directions and accepts it only when both agree.
:::

Alliances declared this way are silent. The announcement, the ceasefire sweep that drops targets already acquired, and the [`AllyReveal`](/keys/allyreveal/) map reveal all belong to alliances formed while a scenario is running, and none of them fires while the scenario is being read.

In a skirmish or multiplayer game the alliances the [launch file](/formats/spawn-ini/#who-is-playing) wrote are made first and kept, and the map's are added to them. A person who allies with somebody during play still makes the computer players [close ranks](/keys/paranoid/) against every person, the map's alliances included, so a cooperative map is best played with `AlliesAllowed=no`.
