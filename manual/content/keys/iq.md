---
key: IQ
summary: A campaign house's intelligence level, which turns on each computer behavior whose threshold it reaches.
see_also: ["system:repair"]
when_omitted:
  kind: value
  value: "0"
---

Each automatic behavior with an entry in the `[IQ]` section turns on for a house whose `IQ` reaches that entry. Raising the value turns the behaviors on in the order of their thresholds. For example, [`RepairSell`](/keys/repairsell/) is the level that turns on [automatic repair and sell-back](/systems/repair/#when-the-computer-repairs).

Three `[IQ]` entries do not compare against this value:

- [`SellBack`](/keys/sellback/) compares against the house's tech level.
- [`ContentScan`](/keys/contentscan/) and [`Aircraft`](/keys/aircraft/) have no effect.

With [`Paranoid`](/keys/paranoid/) on, a computer house whose `IQ` equals [`MaxIQLevels`](/keys/maxiqlevels/) makes the surviving computer houses ally with each other and declare war on the human houses when the [Destroy all of...](/mapping/actions/taction-house-destroy-all/) action defeats it. A computer house that started allied with a human house still in the game is left out.

Keep the value at or below [`MaxIQLevels`](/keys/maxiqlevels/). A larger value is replaced by `1`, not by the maximum, so it turns nearly every behavior off.

Only a campaign mission reads this value. Outside a campaign, every computer house gets the maximum level, and so does a house the computer takes over from a player who leaves.
