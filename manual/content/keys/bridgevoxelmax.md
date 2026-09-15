---
key: BridgeVoxelMax
summary: Parsed debris count that the engine never uses.
no_effect: true
see_also: [BridgeExplosions, MetallicDebris]
when_omitted:
  kind: value
  value: "3"
---

Nothing bounds the debris a collapsing bridge throws. Each cell of a falling section draws its own, and only while [`BridgeExplosions`](/keys/bridgeexplosions/) has at least one entry. Nineteen cells in twenty draw one animation from that list, and half of those also draw one from [`MetallicDebris`](/keys/metallicdebris/).
