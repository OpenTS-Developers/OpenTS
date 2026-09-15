---
key: Warhead
scope: animtype
label: Animation warhead
see_also: ["Damage", "DamageRadius", "ExpireAnim"]
when_omitted:
  kind: value
  value: none
---

The warhead sets [`Damage`](/keys/damage/#scope-animtype) at both points a thrown animation deals it: to each object standing within [`DamageRadius`](/keys/damageradius/#scope-animtype) of a strike, and as the blast where the animation's life ends. Its armor multipliers and its [`Spread`](/keys/spread/#scope-warheadtype) decide what that damage is worth against a given target, and its [`Bright`](/keys/bright/#scope-warheadtype) flag decides whether the impact throws a flash of light.

An animation with no warhead named deals nothing at either point. The blast is refused before it starts, and the strike's damage is reduced to zero. Such an animation still flies, still plays its effects, and still craters or seeds the ground it lands on.

The setting has nothing to do with the damage an ordinary animation applies to the object it is attached to. That path uses the warheads the rules name for it, whatever this setting says.
