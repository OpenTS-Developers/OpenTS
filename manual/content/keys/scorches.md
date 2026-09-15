---
key: Scorches
summary: Parsed smudge list that the engine never uses.
no_effect: true
see_also: ["Burn", "Scorches1", "Craters"]
when_omitted:
  kind: value
  value: ""
---

Scorch marks are not chosen from this list. When an animation that scorches the ground settles on it, the engine gathers every registered smudge type with [`Burn=yes`](/keys/burn/) that will fit the cell. It weighs the size of the marks against the size of the blast and picks one of the survivors at random. A smudge type reaches that pool through its own setting alone, whether or not it is named here.

The section sets five of these lists (this one and [`Scorches1`](/keys/scorches1/) through [`Scorches4`](/keys/scorches4/)), and each is stored in its own place. Nothing reads any of them, and nothing distinguishes one from another.

Naming a smudge the game does not already know still registers one of that name, so an entry here can add an artless smudge type to the game without the list ever placing it.
