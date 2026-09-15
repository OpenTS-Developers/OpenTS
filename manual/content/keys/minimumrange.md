---
key: MinimumRange
summary: How close a target may come before the weapon refuses to fire at it, in cells.
see_also: ["Range", "ProjectileRange"]
when_omitted:
  kind: value
  value: "0"
---

The value is written in cells and a fraction is accepted; it is held as 256 units to the cell. A target whose three-dimensional separation from the firing coordinate falls under it is refused. The refusal is the same one an out-of-range target gets, so an object holding such a target treats it as something to reposition for rather than as an illegal target.

```ini title="rules.ini"
[MyArtillery] ; example WeaponType
Range=12
MinimumRange=4 ; nothing closer than four cells may be shot
```

Repositioning works outward. An object looking for somewhere to fire from starts at its full [`Range=`](/keys/range/) and works inward, taking the first standing spot that satisfies both limits. A vehicle crowded by a target inside its minimum range therefore backs away rather than closing.

The test runs before the shape of the shot is considered, so it holds for an [`Arcing=yes`](/keys/arcing/) projectile as well, which otherwise skips the range comparison entirely. Unlike `Range=`, the figure is compared as written, with nothing taken off it.

A figure of `0` switches the test off. A figure of exactly `-1` is read as though the key were absent, leaving whatever an earlier rules file set. Any other negative figure switches the test off as surely as zero does.
