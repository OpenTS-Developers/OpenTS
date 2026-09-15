---
key: LaserFencePost
summary: Whether the structure anchors laser fence segments run out from it.
see_also: ["system:laser-fences", "system:power"]
when_omitted:
  kind: value
  value: "no"
---

A post looks one cell out in each of the four cardinal directions for a [`LaserFence=yes`](/keys/laserfence/) structure of the same owner whose facing lies along that direction. Each segment a post lays faces along its run, so the segments of a run through this post pass the test, and fence lying across the direction does not. It follows that run to the post at the far end for at most [`GuardRange`](/keys/guardrange/) cells. The key takes a cell count, and a fractional one rounds down; the reach is never fewer than one cell. [Energizing the run](/systems/laser-fences/#energizing-the-run) covers what the search accepts and what a live run does to anything standing in it.

Both posts must be operational for the run to hold, and neither may be going up or coming down, so a shortfall at either end drops the whole thing; [what low power costs](/systems/power/#defenses) covers that test.
