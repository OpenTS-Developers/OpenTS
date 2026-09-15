---
key: RefundPercent
summary: The fraction of an object's price handed back when a human player's house sells it.
see_also: ["system:repair", "system:production"]
when_omitted:
  kind: value
  value: ".5"
---

```ini title="rules.ini"
[General]
RefundPercent=.75   ; a human player's sale returns three quarters of the price
```

The fraction is applied to the price the house would have paid for the type, [multipliers included](/keys/cost/#what-a-house-pays), and the product is truncated to whole credits. Current strength never enters into it, so a structure one point from destruction sells for what an undamaged one does.

Only a house a human is playing takes the reduction. In a campaign that is a house the local player controls: its own, and any the scenario marks [`PlayerControl=yes`](/keys/playercontrol/). In every other game it is a house with a human behind it. Every computer house is refunded the full price instead.

Four paths pay through it:

- selling a structure;
- [selling a vehicle or aircraft at a service depot](/systems/repair/#selling-at-the-pad);
- the upgrade handed back when a structure with one is sold;
- the compensation paid when a construction yard undeploys and its MCV cannot be placed anywhere.

Canceling something still under construction is a different figure and [refunds what has been paid so far](/systems/production/#paying-for-it) in full.
