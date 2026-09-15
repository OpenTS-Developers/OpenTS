---
key: Color
scope: bullettype
label: Voxel projectile remap
see_also: [Inviso, Voxel]
when_omitted:
  kind: computed
  note: The first color scheme in the loaded list.
---

The value is a color scheme name, matched against the loaded schemes without regard to letter case. A scheme with only one intensity level is skipped during that match, and a name matching no remaining scheme leaves the projectile on the scheme it already had.

Only a projectile drawn as a voxel model reads it. The model is posed and rendered, and the finished block is put on screen through this scheme's remap table, so the scheme is what decides the model's remapped colors. A projectile drawn from a shape, or one marked [`Inviso=yes`](/keys/inviso/), never reaches that path and takes no color from here at all.

```ini title="rules.ini"
[ChemMissile]
Image=MISLCHEM ; an art entry marked Voxel=yes
Color=DarkRed
```

Unlike a unit's remap, this is fixed to the projectile type rather than following the house that fired it.
