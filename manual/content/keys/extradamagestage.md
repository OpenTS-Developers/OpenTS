---
key: ExtraDamageStage
summary: Parsed flag that the engine never uses.
no_effect: true
see_also: ["ConditionYellow", "AnimIdle"]
when_omitted:
  kind: value
  value: "yes"
---

The name promises a third condition for the structure's artwork beyond healthy and damaged. Nothing reads the stored flag. A structure's artwork still has exactly two conditions, and [`AnimIdle`](/keys/animidle/) covers where the damaged one begins.

The stored default is `yes`, so writing `ExtraDamageStage=no` changes nothing either.
