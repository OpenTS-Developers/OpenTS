---
key: CrateMoneyBonus
scope: global-rules
label: Random extra money from a crate
summary: The most credits a money crate adds at random to its configured amount.
see_also: [SoloCrateMoney, "system:crates"]
when_omitted:
  kind: value
  value: "900"
---

A money crate pays its configured amount plus a random number of credits from `0` to `CrateMoneyBonus`. At `0` or below it pays the configured amount exactly. The configured amount is the third field of the `Money` entry in `[Powerups]`, or of the `Unit` entry when a unit crate pays money because its vehicle could not be placed.

A campaign crate pays [`SoloCrateMoney`](/keys/solocratemoney/) instead and adds nothing, unless `SoloCrateMoney` is `0`.
