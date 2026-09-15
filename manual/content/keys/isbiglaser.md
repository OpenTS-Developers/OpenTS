---
key: IsBigLaser
summary: Widens the screen glow drawn along a laser beam.
see_also: ["IsLaser", "LaserDuration"]
when_omitted:
  kind: value
  value: "no"
---

A laser shot draws two things: the colored beam itself and a glow spanning the same line, which raises the red of whatever terrain and objects lie beneath it. This flag chooses between the two sizes of that glow. The wide one is 88 leptons across and the ordinary one 68, against a cell's 256 leptons. It is also drawn a little shorter, stopping 34 leptons short of each end of the beam against the ordinary one's 27.

```ini title="rules.ini"
[MyObeliskRay] ; example WeaponType
IsLaser=yes
IsBigLaser=yes
```

The glow fades on its own schedule of about 22 frames and takes no notice of [`LaserDuration`](/keys/laserduration/), which governs only the colored beam. Nothing here changes the beam's colors, its damage or its reach. The flag is read only when [`IsLaser=yes`](/keys/islaser/) puts a beam on screen and, like the rest of the beam settings, comes from the weapon in the object's first slot rather than from the slot that fired.
