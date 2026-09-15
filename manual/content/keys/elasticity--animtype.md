---
key: Elasticity
scope: animtype
label: Animation bounciness
see_also: ["Bouncer", "BounceAnim", "BounceSound", "DamageRadius"]
when_omitted:
  kind: value
  value: "0.8"
---

The multiplier applied to a thrown animation's velocity when it reaches a surface. The velocity is rotated into the local space of the slope it struck, scaled by this figure, flipped vertically, and rotated back out. The setting therefore decides how much speed survives the contact, and nothing about the direction the animation leaves in.

A [`Bouncer=yes`](/keys/bouncer/) or [`IsMeteor=yes`](/keys/ismeteor/#scope-animtype) animation is removed on its first contact. The rebound is computed and then thrown away with the animation, so the surviving speed has no second leg to carry it through. What the setting really decides is how that one contact is classified. The speed left afterward and the height the animation is still standing off the ground are combined into a single figure. The height is converted into an equivalent downward speed at the bounce gravity, 1.4 per frame for every lepton of height, and added to the remaining vertical speed. The horizontal speed and that vertical part then combine as the sides of a right triangle. A figure below `2.5` is reported as a settling rather than a strike.

That distinction is worth exactly three effects. [`BounceAnim`](/keys/bounceanim/#scope-animtype), [`BounceSound`](/keys/bouncesound/#scope-animtype) and the [`DamageRadius`](/keys/damageradius/#scope-animtype) damage all hang off a strike and are skipped where the animation settles instead. The impact effects (the expiry animation, the blast, the impact sound, the spawns and any Tiberium) run either way.

:::caution[Zero suppresses the strike effects on a landing]
An animation that keeps none of its speed has nothing left to carry it back off the ground, so coming down on the ground, onto a bridge deck, or against a building or wall is reported as settling. Every bouncing animation in the shipped `art.ini` declares `Elasticity=0.0`, so a landing that puts one of them on the ground or on a deck plays no bounce animation or sound and deals no bounce damage. A contact that leaves the animation off that level still counts the standing height and is reported as a strike. That happens against a cliff face, or up through the underside of a bridge.
:::
