---
key: PowerEmergency
summary: Parsed power fraction that the engine never uses.
no_effect: true
see_also: ["system:ai-base-building", "system:power", PowerSurplus]
when_omitted:
  kind: value
  value: ".75"
---

No routine sells a computer house's buildings to recover power. A shortfall is answered while the plan is walked, by [inserting a power plant node ahead of the structure that would cause it](/systems/ai-base-building/#power-and-money-interventions). The only sell-off in the planner is the one a house that cannot earn performs to pay for a harvester or a refinery.
