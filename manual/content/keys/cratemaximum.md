---
key: CrateMaximum
summary: Most crates placed at the start of a non-campaign scenario.
see_also: ["system:crates"]
when_omitted:
  kind: value
  value: "255"
---

The setting clamps the starting count computed from [`CrateMinimum`](/keys/crateminimum/) and the number of human players, and nothing else reads it. Replacements placed on expiry and on pickup are not counted against it, so it is not a cap on how many crates the map holds at once.

That cap comes from the 256 tracking slots the engine keeps: a placement request that finds every slot in use places nothing. Setting this above 256 therefore changes nothing.
