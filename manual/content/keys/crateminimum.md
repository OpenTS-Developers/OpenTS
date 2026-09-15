---
key: CrateMinimum
summary: Fewest crates placed at the start of a non-campaign scenario.
see_also: ["system:crates"]
when_omitted:
  kind: value
  value: "1"
---

The starting count is the larger of this setting and the number of human players the match was set up with, then clamped down by [`CrateMaximum`](/keys/cratemaximum/). It applies only to the one placement pass that runs as a non-campaign scenario finishes loading. The replacements placed later, on expiry and on pickup, come one at a time and read neither limit.

Each of those starting crates still has to find a legal cell, and [Where a random crate can land](/systems/crates/#where-a-random-crate-can-land) covers the cases in which the search produces no visible crate.
