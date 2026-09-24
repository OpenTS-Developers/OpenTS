---
key: SoloCrateMoney
summary: Credits a campaign crate pays when its result is money.
see_also: ["system:crates"]
when_omitted:
  kind: value
  value: "2000"
---

The figure is fixed and replaces the random payout used outside a campaign. Setting it to `0` restores that random range, which pays between the money result's own configured figure and [`CrateMoneyBonus`](/keys/cratemoneybonus/) credits more than it.

In a campaign the credits go to the house the local player commands whenever the collecting object belongs to a house the player drives, rather than to that collecting house. The two are the same house in an ordinary mission and separate only where a mission gives the player a second one.
