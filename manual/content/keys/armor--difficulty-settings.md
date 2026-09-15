---
key: Armor
scope: difficulty-settings
label: Difficulty damage divisor
see_also: ["system:difficulty"]
when_omitted:
  kind: value
  value: "1"
  note: The difficulty block is re-read from fixed defaults whenever its section is present, so a later file that contains the section without this key restores 1 rather than keeping the earlier value.
---

`[Easy]`, `[Normal]` and `[Difficult]` each set their own figure, and a house takes the one for [the difficulty slot it is assigned](/systems/difficulty/#from-the-setting-to-a-slot). Damage arriving at one of that house's objects is divided by this figure together with the object's own armor multiplier, not multiplied, so a figure above 1 makes the house tougher and one below 1 more fragile. The division runs only on positive damage the source has not forced through. It lands ahead of the veteran armor bonus and of type immunity, and the result of both reductions is floored at 1, so no figure here can bring incoming damage to nothing.

The product is worked out once, when the house is given its slot, alongside the [country's own figure](/keys/armor/#scope-housetype) outside campaign games. In a campaign game the country's figure is dropped and this one stands alone.

:::caution[A zero makes the house nearly invulnerable]
The figure is a divisor. The difficulty sections start at zero and are read only out of a file that contains them. A rules tree in which one of the three sections never appears leaves that slot at zero, as does assigning `0` directly. Every hit a house in that slot takes then divides by zero, and the result lands below the floor the engine applies afterwards. Each hit costs exactly 1 point of strength however heavy the shot was.
:::
