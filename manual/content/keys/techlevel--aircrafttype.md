---
key: TechLevel
scope: aircrafttype
label: Object tech level
see_also: ["system:production"]
when_omitted:
  kind: value
  value: "255"
---

A house may build the type only while [the level it holds](/keys/techlevel/#scope-house-per-scenario) is at least this value. A value of `-1` blocks the type for every house whatever level it holds, which is how a type stays defined without ever being buildable.

This is the first of the [build-list gates](/systems/production/#what-a-house-may-build) and the only one a computer house is also put through. The stored default sits far above the levels houses are given in practice. A type that never names the key is therefore effectively unbuildable rather than universally available.
