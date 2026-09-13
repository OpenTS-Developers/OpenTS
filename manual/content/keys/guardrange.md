---
key: GuardRange
summary: The distance in cells an object scans for targets while guarding, overriding its weapon range.
see_also: ["system:target-selection"]
when_omitted:
  kind: value
  value: "0"
  note: Zero is the instruction to use the object's own weapon range instead, so an omitted value costs nothing.
---

The value is written in cells and fractions are accepted. It supplies all three scan radii: the guard radius is this value outright, while the area radius used by Guard Area and the patrol radius used by Patrol are twice it, the first clamped to at most 16 cells and the second to between 7 and 16. At `0` the area and patrol radii fall back to twice the longer of the object's two weapon ranges, while the guard radius becomes a live test against the range of the weapon chosen per candidate.

```ini title="rules.ini"
[MYTANK] ; example UnitType
GuardRange=7.5
```

An engineer ignores the guard radius and always scans at weapon range. An object with no first-slot weapon has no range test to run and uses this value as its acceptance distance instead.

Two leash distances come off the same figure. A Guard Area object that is neither firing nor already traveling somewhere breaks off and heads home once it stands more than three quarters of the area radius from its home position. A patrolling object accepts a target it cannot already shoot only when the walk to it is shorter than the patrol radius in cells plus six.

:::caution[The value is also a fence connection distance]
A [`LaserFencePost=yes`](/keys/laserfencepost/) building uses it as the number of cells it searches in each of the four directions for its neighboring post, treating anything under one cell as one. A [`FirestormWall=yes`](/keys/firestormwall/) type uses it as the number of cells a newly placed section searches for another section to join to, with no such floor. Both truncate it to whole cells, so changing it on those types changes how far a fence run reaches.
:::

A value of exactly `-1` is indistinguishable from omitting the key: the reader treats it as absent and keeps whatever was already stored.
