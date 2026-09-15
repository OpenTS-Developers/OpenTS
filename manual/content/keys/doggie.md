---
key: Doggie
summary: Gives the soldier the attack dog's panic, its habit of bedding down in Tiberium, and its own death.
see_also: [Fraidycat, Fearless, ConditionRed, DeadBodies, InfDeath, "system:tiberium"]
when_omitted:
  kind: value
  value: "no"
---

Fear is a figure from `0` to `255` that a soldier accumulates as it is shot at. A dog that is frightened by a hit while its health is at or below [`ConditionRed`](/keys/conditionred/) jumps straight to `200`, full panic, where any other soldier would stop at `100`.

From there a dog's fear behaves nothing like a soldier's. While it is at `200` or above, standing still and with nowhere to go, a dog standing on Tiberium lies down. One that is not looks for Tiberium within 16 cells and walks to it. It then leaves the routine, so none of the ordinary fear handling applies to it. It does not stand up as it calms, it does not drop prone at the first flicker of fear, and it never reaches the reload an armed civilian gets when its fear returns to zero. The fear itself still works off at one point per logic frame unless the type also declares [`Fearless=yes`](/keys/fearless/).

Bedding down also happens outside panic. A dog on Guard with no target, no destination and no turn in progress, standing on Tiberium and not already prone, turns to face east and then lies down. That is how dogs come to be found asleep in the fields. A prone dog given a destination stands up again.

Death is separate again. Where a warhead's [`InfDeath`](/keys/infdeath/) would set an ordinary soldier alight or electrocute it, a dog plays its own burning death animation instead. When any death animation finishes, a dog is removed without leaving one of the [`DeadBodies`](/keys/deadbodies/) corpses behind.
