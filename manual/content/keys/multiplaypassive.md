---
key: MultiplayPassive
summary: Keeps the country's computer houses from building or choosing attacks and, in skirmish and multiplayer games, takes its houses out of the contest.
see_also: [Multiplay, WallOwner, Allies]
when_omitted:
  kind: value
  value: "no"
---

```ini title="rules.ini"
[Neutral]
MultiplayPassive=true
```

In any kind of game, a computer-controlled house of a passive country never chooses attacks and never queues buildings, vehicles, infantry or aircraft. It raises teams from AI triggers only after [its AI-trigger switch](/systems/ai-team-production/#when-the-pass-runs) is turned on, for example by a map trigger action.

The structures of a passive country's houses never pay [`ProduceCashAmount`](/keys/producecashamount/) income, whoever controls the house.

In skirmish and multiplayer games, a passive house is treated as scenery, not as an opponent:

- It is exempt from the defeat check that removes a player who has run out of objects, and its defeat is never announced.
- It is not counted among the players still alive, and the test for whether every remaining player is allied skips it.
- A computer house never picks it as the nearest enemy, but damage from its objects can still make it that house's enemy.
- No house can break an alliance with it or declare war on it.
- Automatic target scans reject its objects unless the [launch file](/formats/spawn-ini/) sets `AttackNeutralUnits=`.
- Its objects never collect crates and do not reveal the map.
- Its structures are never marked for automatic repair.
- The score screen and the starting-unit generator skip it.

An [observer's](/systems/observers/) house is also left out of these counts, lists and scores, whatever its country.

Skirmish and multiplayer setup also sets [`WallOwner=no`](/keys/wallowner/#scope-housetype) on the country of every passive house in the game and `WallOwner=yes` on every other house's country. Map walls already have their owners by then, so this changes no wall's owner. Setup then allies every house in the game with the house of the country named `Special`.

:::caution[Keep the `Neutral` and `Special` countries]
Skirmish and multiplayer setup creates a house for the countries named `Neutral` and `Special` by name and does not check that either country exists. Keep both countries in the rules for skirmish and multiplayer games.
:::
