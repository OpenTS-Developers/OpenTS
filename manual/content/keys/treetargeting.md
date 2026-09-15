---
key: TreeTargeting
summary: Whether the attack cursor appears over trees without the force-fire modifier held.
see_also: ["LegalTarget"]
when_omitted:
  kind: value
  value: "no"
---

The setting reaches the cursor and nothing else. With it off, an object the player has selected offers to attack a terrain object only while the force-fire modifier is held, or where that terrain type sets [`LegalTarget=yes`](/keys/legaltarget/). With it on, every terrain object is offered whatever its own setting says.

It does not widen what the engine will fire at once the order is given, and it does not enter into what a computer-controlled house chooses to shoot.
