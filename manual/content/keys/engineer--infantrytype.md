---
key: Engineer
scope: infantrytype
label: Engineer soldier
see_also: ["system:capture"]
when_omitted:
  kind: value
  value: "no"
---

Setting this also forces [`Infiltrate=yes`](/keys/infiltrate/) on, in the same pass and after that key has been read, so a section that sets `Engineer=yes` cannot write `Infiltrate=no`. The forcing is not undone. A later rules layer that writes `Engineer=no` over the type clears the engineer behavior and leaves the infiltrate behavior standing. The result is a soldier that still takes the enter cursor, still walks into the structure, and does nothing when it arrives.

The flag drives its own [cursor block](/systems/capture/#an-engineer-over-a-structure), the restore-or-capture split at the structure, and the bridge repair branch. It also changes how the soldier travels and what its owner points it at. An engineer is given its target as a movement destination instead of closing to weapon range. It may walk into an occupied cell while on guard, area guard or patrol, and it skips target scanning on plain guard. [Target selection](/systems/target-selection/#what-each-kind-of-object-considers) treats it as a special case throughout.
