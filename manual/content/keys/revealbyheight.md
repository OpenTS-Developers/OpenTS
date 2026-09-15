---
key: RevealByHeight
summary: Whether high ground between an object and a cell blocks that cell from being revealed.
see_also: ["system:map-visibility", Sight]
when_omitted:
  kind: value
  value: "yes"
---

At `yes` each cell inside the sight radius is checked against one probed cell before it is revealed. The probe is the candidate cell's ground cell, displaced two cells on both axes and then stepped once toward the object doing the looking. The reveal is refused while that probed ground stands more than three height levels above that object. At `no` the check is skipped and every cell inside the sight radius is revealed regardless of the terrain between.

Switching it off also turns on a second behavior. An object that has moved one cell can rescan the outer rings of its sight radius rather than the whole disc, and that shortcut is guarded on this flag being `no`. At the default every look scans the full disc.

[The scan](/systems/map-visibility/#the-scan) covers both, including which cell is probed.
