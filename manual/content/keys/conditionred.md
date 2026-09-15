---
key: ConditionRed
summary: The fraction of maximum strength at or below which an object counts as critically damaged.
see_also: ["system:repair"]
when_omitted:
  kind: value
  value: ".5"
---

Crossing the threshold turns an object's health bar and its selection pips red. An airborne aircraft below it trails hard-coded `SGRYSMK1` smoke. The computer also applies it as the health test when it decides to [sell a damaged building instead of repairing it](/systems/repair/#when-the-computer-repairs). A patrolling engineer drops an allied building as a renovation target once the building falls to it or below.

The engine default is the same fraction as [`ConditionYellow`](/keys/conditionyellow/). At the defaults an object reaches both conditions at the same moment, and the yellow band is empty.
