---
key: IQ
summary: The house's intelligence level, which opens each computer behavior whose own threshold it reaches.
see_also: ["system:repair"]
when_omitted:
  kind: value
  value: "0"
---

Nearly every automatic behavior with an entry in the `[IQ]` section compares that entry against this number, so raising it switches those behaviors on in the order their thresholds fall. Two entries are exceptions: [`SellBack`](/keys/sellback/) compares against the house's tech level instead, and [`ContentScan`](/keys/contentscan/) is compared in a routine nothing calls. [`RepairSell`](/keys/repairsell/) is the threshold that turns on [automatic repair and sell-back](/systems/repair/#when-the-computer-repairs).

Two rules override what a scenario writes here. A value above [`MaxIQLevels`](/keys/maxiqlevels/) is replaced by `1`, not by the maximum, so an over-large number switches nearly everything off rather than on. Outside campaign games the assignment is never read at all: the engine gives every computer house the maximum level.
