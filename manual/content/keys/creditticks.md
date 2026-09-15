---
key: CreditTicks
summary: The two sounds the credit readout plays as it counts, the first while it rises and the second while it falls.
see_also: ["system:sidebar"]
when_omitted:
  kind: value
  value: ""
  note: The list stays empty and both positions are still indexed whenever the readout moves, dereferencing a pointer that was never given storage.
---

The readout never jumps to the house's money. It walks toward it a step at a time and plays one of these sounds at half volume on every step. A single transaction is heard as a run of ticks rather than one. [The credit readout](/systems/sidebar/#the-credit-readout) covers how far each step moves and how often one is taken.

:::danger[An empty list crashes the readout]
Only the first two positions are read, and they are read without a length check. A list of one entry is indexed past its end, and an empty list holds no storage at all, so the read dereferences a null pointer the first time the player's money changes. Omitting the key leaves exactly that empty list, so every rules tree needs two entries here. This is one of the few settings with no safe absent state.
:::
