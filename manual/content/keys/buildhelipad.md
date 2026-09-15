---
key: BuildHelipad
summary: Parsed helipad list for a computer base that the engine never uses.
no_effect: true
see_also: ["system:ai-base-building", HelipadLimit, Helipad]
when_omitted:
  kind: value
  value: ""
---

What makes a building a helipad to the planner is [`Helipad=yes`](/keys/helipad/) on the type itself, which is what earns it extra copies while [the plan is assembled](/systems/ai-base-building/#building-the-plan). This list is read from the rules, and nothing reads it.
