---
key: Anim
summary: The animations played at the firing coordinate when the weapon goes off, either one fixed entry or one per facing.
see_also: ["Report", "PrimaryFireFLH"]
when_omitted:
  kind: value
  value: ""
---

The list length decides how it is spent. A length of `8`, `16`, `32` or `64` makes the choice directional, one entry per facing; any other non-empty length uses the first entry for every shot and ignores the rest. The animation is created at the muzzle coordinate the shot left from, and on anything other than a building it is attached to the firing object so that it travels with it.

```ini title="rules.ini"
[MyGatling] ; example WeaponType
Anim=GUNFIRE1,GUNFIRE2,GUNFIRE3,GUNFIRE4,GUNFIRE5,GUNFIRE6,GUNFIRE7,GUNFIRE8 ; AnimTypes registered in [Animations]
```

The order starts at north-west rather than at north, so an eight-entry list runs north-west, north, north-east, east, south-east, south, south-west, west. A longer list starts at north-west too and steps round in as many parts as it holds. The facing tested is the direction the weapon is currently aiming. A vehicle uses its turret's facing where it has one and its body's facing otherwise; an infantry and an aircraft always use the body's facing; a structure uses its turret's facing, but falls back to the direction of its target when it has no turret or its turret animation is a voxel.

Writing `Anim=none` empties the list and leaves the weapon with no firing animation, because the placeholder resolves to no type and nothing is added. Writing the key with nothing after the `=` is a different thing: the read finds no value and keeps whatever an earlier rules file set. A name the game does not already know is registered as a new animation type rather than rejected, so a misspelled entry becomes a blank animation of its own and still occupies its place in the list.

The list is spent only when an object fires the weapon at a target. A detonation staged by the [Do Explosion At](/mapping/actions/taction-do-explosion/) trigger action, and the shot an EM pulse cannon throws at its stored destination, both skip it and leave the muzzle bare.
