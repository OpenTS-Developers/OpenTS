---
key: Aircraft
summary: Parsed intelligence level that the engine never uses.
no_effect: true
see_also: ["system:ai-team-production", IQ, Harvester]
when_omitted:
  kind: value
  value: "4"
---

Aircraft are not replaced on their own account, and no intelligence level gates them. A computer house orders one through [the same demand tally that chooses its vehicles and infantry](/systems/ai-team-production/#production-demand), which counts the places its teams cannot fill and never reads the house's [`IQ`](/keys/iq/). The one replacement the intelligence scale really does gate is [`Harvester`](/keys/harvester/#scope-global-rules).
