---
key: MaxXYVel
scope: voxelanimtype
label: Voxel debris lateral speed
see_also: ["MinZVel", "MaxZVel", "IsMeteor"]
when_omitted:
  kind: value
  value: "15"
---

Leptons per frame: 256 leptons to a cell, 15 frames to the second. The two horizontal components of the launch velocity are drawn independently, each a whole number taken from twice this figure and then shifted down by it. A whole-number setting therefore gives speeds from minus the setting up to one lepton per frame short of it, in either axis, so the positive extreme is never reached.

A meteor draws both components from the same range and then mirrors the pair whenever `X` falls below `-Y`, which is what confines its approach to one half of the compass. [`IsMeteor`](/keys/ismeteor/#scope-voxelanimtype) describes the full flight.

:::danger[A small setting divides by zero]
The pick divides by the truncated whole number of twice this figure, so any value between `-0.5` and `0.5` (`0` among them) makes that divisor zero. The division faults, and the game stops the moment a piece of the type is created.
:::
