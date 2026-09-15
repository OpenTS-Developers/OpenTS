---
key: SurvivorRate
summary: The fraction of a structure's cost that decides how many survivors leave it.
see_also: ["system:capture"]
when_omitted:
  kind: value
  value: ".5"
---

The count is the cost the structure's owner pays for it, multiplied by this fraction and divided by [`SurvivorDivisor`](/keys/survivordivisor/), then clamped to between 1 and 5. The clamp means this setting can only move a structure inside that band. The count is worked out on destruction and on a sale alike, and only for a [`Crewed=yes`](/keys/crewed/) type. [Survivors](/systems/capture/#survivors) covers the per-cell odds a destroyed structure rolls the count against, and what a capture and an armed demolition charge do to them.
