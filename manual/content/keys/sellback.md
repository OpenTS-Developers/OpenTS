---
key: SellBack
summary: The tech level a house must have reached before the computer will sell a building it cannot afford to repair.
see_also: ["system:repair"]
when_omitted:
  kind: value
  value: "2"
---

Despite living among the intelligence settings, the comparison is against the house's tech level rather than its [`IQ`](/keys/iq/). The same tech level is then drawn against: a random number from `0` through `50` inclusive must come up below it. A house at tech level 5 therefore goes through with the sale on roughly one consideration in ten, and only a tech level above 50 never fails the draw.

Among the further conditions are a shortfall against [`CreditReserve`](/keys/creditreserve/), damage from a non-ally, no trigger tag, and health below [`ConditionRed`](/keys/conditionred/); the full list is traced under [when the computer repairs](/systems/repair/#when-the-computer-repairs).
