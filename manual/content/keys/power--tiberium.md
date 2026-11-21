---
key: Power
scope: tiberium
label: Tiberium damage
when_omitted:
  kind: value
  value: "0"
  note: A damage figure of zero.
---

The value sets the damage this Tiberium deals in the four cases below.

Infantry take `Power / 10` damage, rounded down and at least 1, each time they finish moving into a cell of this type. A Tiberium type can replace that damage amount with [`DamageToInfantry`](/keys/damagetoinfantry/). Infantry whose type sets [`TiberiumProof=yes`](/keys/tiberiumproof/), or that have the Tiberium-proof [veteran ability](/systems/veterancy/#abilities), take no damage. The damage uses [`C4Warhead`](/keys/c4warhead/) and is forced, so armor and [`Immune=yes`](/keys/immune/#scope-aircrafttype) do not reduce it.

A cell that detonates in a [chain reaction](/systems/tiberium/#damage) loses half its growth stages, rounded down, and deals that many stages multiplied by this value through `C4Warhead`. At a result of zero the detonation shows no explosion and deals no damage, but it still removes the stages and can still set off neighboring cells.

With [`TiberiumExplosive=yes`](/keys/tiberiumexplosive/#scope-global-rules) in `[CombatDamage]`, a destroyed vehicle carrying Tiberium, such as a harvester, explodes through `C4Warhead` over a radius of one and a half cells. The damage is the sum, over every Tiberium type it carries, of the amount carried multiplied by that type's `Power`. The blast goes off only if the vehicle has a death explosion animation, such as one from its [`Explosion`](/keys/explosion/) list. A scenario with [`HarvesterImmune=yes`](/keys/harvesterimmune/) prevents it. [Spilled harvester loads](/systems/destruction-and-debris/#spilled-harvester-loads) covers which deaths set it off.

A destroyed structure that is [`Explodes=yes`](/keys/explodes/#scope-aircrafttype), or has the explodes [veteran ability](/systems/veterancy/#abilities), adds the same sum for the Tiberium in its [`Storage`](/keys/storage/) to its [collateral figure](/keys/collateraldamagecoefficient/). That blast uses the warhead of the structure's primary weapon, not `C4Warhead`. A structure with no primary weapon deals no blast damage, so its stored Tiberium adds nothing.
