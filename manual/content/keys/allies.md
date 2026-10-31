---
key: Allies
summary: The houses a scenario's house counts as friendly.
see_also: [Owner, AllyReveal, Edge]
when_omitted:
  kind: value
  value: ""
  note: An empty list, which leaves the house allied to nobody but itself.
---

The value is a comma-separated list of houses that this house treats as friends. An allied object is [rejected by an automatic target scan](/systems/target-selection/#why-a-candidate-is-rejected), is passed over instead of crushed, and does not provoke [retaliation](/systems/target-selection/#retaliation). A house is always its own ally, whether or not it names itself.

## Where the list is read

A campaign mission reads `Allies` from each house record. Each name matches a country by its section name or its `Name=`, ignoring letter case, and allies the house with that country's house. Write the names with no space after the commas: in a house record, a name with a leading space matches nothing.

```ini title="scenario map file"
[Special] ; a house record in the scenario's own house list
Allies=GDI,Nod
```

A skirmish or multiplayer game reads `Allies` from the [spawn house](/formats/scenario-objects/#spawn-houses) sections `[Spawn1]` through `[Spawn8]` instead. Each section applies to the house that starts at that position, and a section for a position nobody holds is ignored. Each name can be one of these:

- A spawn house, such as `Spawn2`, allies with the one house that starts at that position.
- A country allies with every house playing that country.

An observer is never allied.

```ini title="multiplayer map file"
[Spawn1] ; whoever starts at waypoint 0
Allies=Spawn2,Nod
```

Alliances from the [launch file](/formats/spawn-ini/#who-is-playing) are made first and kept, and the map's alliances are added to them.

:::caution[The alliance runs one way]
Naming a house here makes this house treat that house as a friend, not the reverse. For a mutual truce, name each house in the other's list.
:::

When a house is defeated in a skirmish or multiplayer game, or by the [Destroy all of...](/mapping/actions/taction-house-destroy-all/) action, the [match ends](/systems/observers/#when-the-match-ends) if every house left is allied with every other in both directions.

## What a map alliance does not do

Alliances from the map take effect without the alliance announcement. They also skip two things that happen when an alliance forms during play: the house's units drop targets that are now friendly, and, with [`AllyReveal`](/keys/allyreveal/) on, a house that allies with the local player reveals the terrain around all its objects at once.

With [`Paranoid`](/keys/paranoid/) on, the computer houses close ranks when a human house forms an alliance during play, and when a computer house whose IQ equals [`MaxIQLevels`](/keys/maxiqlevels/) is defeated. Every computer player in a skirmish or multiplayer game has that IQ. A computer house that was allied with a human house when the scenario was set up keeps that alliance while the human house is still in the game. An alliance a computer house forms after the match has started is broken. `Paranoid=no` stops the computer houses from closing ranks. Leaving `AlliesAllowed` at `no` in the launch file's [`[Settings]`](/formats/spawn-ini/#the-options-every-house-plays-under) stops only the alliances players form during play.
