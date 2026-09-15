---
key: Scorch
summary: Blackens the ground under the animation and sets a small fire burning there.
see_also: ["Crater", "Flamer", "Scorches", "SmallFire"]
when_omitted:
  kind: value
  value: "no"
---

Both effects hang off the frame whose artwork covers the most ground, and both run again each time a looping animation comes back to that frame.

## The scorch mark

While the animation stands less than 30 leptons above the ground beneath it, a scorch is stamped at its position. The mark is picked at random from the smudge types declaring [`Burn=yes`](/keys/burn/). The location fits only when **none of** the following is true:

- the cell lies outside the playfield;
- the ground has a ramp;
- the cell has a smudge already;
- the cell holds an overlay;
- a building stands on the cell;
- the ground is a tile kind that will not take a smudge ([`Morphable=no`](/keys/morphable/)).

Multiple cell scorches join the candidates only where that largest frame measures more than 48 pixels across and 40 down.

An animation declaring [`Crater=yes`](/keys/crater/#scope-animtype) as well leaves a scorch half the time and a crater the other half, never both. This setting is the one tested first, so the crater is the fallback.

## The fire

Below 10 leptons above the ground, the animation also creates the rules' [`SmallFire`](/keys/smallfire/) at its own position. The fire plays for one or two times the loop count the fire type sets. The animation hands it to whatever object it is riding, so the two come off together. Water, beach, ice and rock take no fire; the scorch mark above is not filtered that way.

[`Flamer=yes`](/keys/flamer/) displaces the fire entirely. An animation declaring both still lays its scorch mark, but throws the flame thrower's scattered fires instead of this single one.
