---
title: Read the overridden type values from the rules
category: balance
release: 0.2.0
targets:
- type: key
  id: BaseNormal
  effect: changed
- type: key
  id: Strength
  effect: changed
- type: key
  id: Cost
  effect: changed
- type: key
  id: Explodes
  effect: changed
- type: key
  id: GuardRange
  effect: changed
breaking: true
migration:
- 'Set `[HMEC] Strength=1200` in the rules to keep the Mammoth Mk. II at the strength it had. Stock data says `800`, which is now what it gets.'
- 'Set `Cost=250` in `[GAFSDF]`, `[GAWALL]` and `[NAWALL]` to keep the wall prices, and the build times derived from them. Stock data says `50`.'
- 'Add `[E2] Explodes=yes` to keep the Disc Thrower exploding on death. Stock data leaves the key out, which means no.'
- 'Add `[NAFNCE] BaseNormal=no` to keep laser fence sections from anchoring building placement. Stock data carries `IsBase=no`, which the engine has never read, so without this the section becomes a valid anchor.'
credit:
- ZivDero
---

Seven object types no longer have values rewritten in code after their section
is read. [`Strength`](/keys/strength/), [`Cost`](/keys/cost/),
[`Explodes`](/keys/explodes/), [`BaseNormal`](/keys/basenormal/) and
[`GuardRange`](/keys/guardrange/) now decide what those types carry, in every
game type.

The affected IDs are `HMEC`, `GAFSDF`, `GAWALL`, `NAWALL`, `E2`, `NAFNCE` and
`NAPOST`. Unlike the multiplayer weapon overrides, these were applied in
campaigns too, so the migration below matters to single-player as well.

Two of the overrides were already writing the value the rules gave them, so they
need no migration. All three wall sections carry `GuardRange=5`, which is the
five cells the code pinned, and `[NAPOST]` already carries `BaseNormal=no`.
