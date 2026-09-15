---
key: Craters
summary: Parsed smudge list that the engine never uses.
no_effect: true
see_also: ["Crater", "Scorches"]
when_omitted:
  kind: value
  value: ""
---

Craters are not chosen from this list. When an animation that forms craters settles on the ground, the engine gathers every registered smudge type with [`Crater=yes`](/keys/crater/#scope-smudgetype) that will fit the cell. The mark's size is the smudge type's declared `Width` and `Height` in cells; the blast's is the drawn size of the settling animation. A small blast picks from the one-cell craters alone, falling back on bigger marks only when no one-cell crater fits. A big blast leaves every crater that fits in the running. One of the survivors is picked at random. A smudge type reaches that pool through its own setting alone, whether or not it is named here.

Naming a smudge the game does not already know still registers one of that name, so an entry here can add an artless smudge type to the game without the list ever placing it.
