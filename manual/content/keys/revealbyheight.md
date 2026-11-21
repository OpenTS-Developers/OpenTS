---
key: RevealByHeight
summary: Whether high ground between an object and a cell blocks that cell from being revealed.
see_also: ["system:map-visibility", Sight]
when_omitted:
  kind: value
  value: "yes"
---

With `yes`, high ground can hide cells from a look. Each cell in the sight radius is revealed only if the ground at one nearby probe cell is no more than three height levels above the object looking. With `no`, every cell in the sight radius is revealed, whatever the terrain.

[The scan](/systems/map-visibility/#the-scan) shows which cell is probed.
