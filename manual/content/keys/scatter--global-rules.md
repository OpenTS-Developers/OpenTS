---
key: Scatter
scope: global-rules
label: IQ threshold
see_also: [IQ, PlayerScatter]
when_omitted:
  kind: value
  value: "3"
---

A cell that is warned a threat is coming asks each of its occupants whether it should get out of the way. One of the answers that says yes is the occupant's house holding an [`IQ`](/keys/iq/) at or above this level. The comparison is against the house, not the object, and it never asks whether that house is under computer control. A map that raises a player house's `IQ` this far has its vehicles stepping aside from incoming fire without being told to.

The threshold is only one of several ways past that decision, and the same warning reaches other objects through the others; [`PlayerScatter`](/keys/playerscatter/) covers the full test and the two further decisions that read it.

:::caution[A moving soldier of such a house scatters differently]
A soldier that is already moving has the forced mark cleared, so the refusal that spares a person's infantry catches it whatever the warning says, and it keeps to its path. A standing soldier is moved like the vehicles when the warning is forced, and is spared when it is not.
:::
