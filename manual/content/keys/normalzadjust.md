---
key: NormalZAdjust
summary: The depth bias applied to the structure's main shape.
see_also: ["ZShapePointMove", "DeployingAnim", "SpecialZOverlayZAdjust"]
when_omitted:
  kind: value
  value: "0"
---

The figure biases the depth the structure's own artwork is tested at, in the same direction as an animation's depth bias. A negative figure brings the structure toward the viewer so that it covers more of what surrounds it. A positive one pushes it away.

```ini title="art.ini"
[MYWEAP] ; example war factory, drawn from its own Image ID
NormalZAdjust=-10
```

It applies to the structure's main shape. The [`BibShape`](/keys/bibshape/) apron and the [`DoorAnim`](/keys/dooranim/) frames each have a fixed bias of their own, one pixel and five pixels toward the viewer, and the [`UnderDoorAnim`](/keys/underdooranim/) shape has none. The bias is dropped to zero for the frames drawn from [`DeployingAnim`](/keys/deployinganim/) while the structure is unloading. It is passed over altogether on a [`FirestormWall=yes`](/keys/firestormwall/) structure. A [`LaserFence=yes`](/keys/laserfence/) structure passes it over on the two frames it draws at rest, one for each placement axis, and both of those take a fixed one pixel toward the viewer instead. The frames a [`Gate=yes`](/keys/gate/) structure draws while its gate is moving keep it, and so does the copy of a structure drawn under fog.
