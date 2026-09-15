---
key: AttachedSystem
summary: The particle system created with a voxel animation and destroyed with it.
see_also: ["TrailerAnim", "Duration", "StartSound"]
when_omitted:
  kind: value
  value: none
---

One particle system of the named type is created as the piece is created and disposed of as the piece is removed. It is placed at the coordinate the piece was asked for (for a meteor, the coordinate it is aiming at rather than the point it starts from) and records the piece as its source.

Only a system declaring [`BehavesLike=Smoke`](/keys/behaveslike/) acts on that record: it re-centers itself on the piece every frame, so the plume travels with the debris. Every other behavior stays at the coordinate it was created at, however far the piece flies. A system declaring `BehavesLike=Fire` goes further and deletes itself on its first frame of logic. That behavior needs a source that can fire a weapon, and a piece of debris is not one.

A value naming no registered particle system is not refused: a system type of that name is created on the spot, with every setting at its built-in value.

A trail built from animations rather than particles is [`TrailerAnim`](/keys/traileranim/#scope-voxelanimtype) instead.
