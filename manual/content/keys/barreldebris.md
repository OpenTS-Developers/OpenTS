---
key: BarrelDebris
summary: The voxel wreckage thrown clear when an exploding overlay is set off.
see_also: [AmmoCrateDamage, BarrelExplode, BarrelParticle, Explodes]
when_omitted:
  kind: value
  value: ""
---

```ini title="rules.ini"
[AudioVisual]
BarrelDebris=MYTANKPIECE,MYSHRAPNEL ; VoxelAnimTypes registered in [VoxelAnims]
```

[`AmmoCrateDamage`](/keys/ammocratedamage/) covers the explosion this wreckage belongs to. The list is walked from the front and each entry is offered a fifteen percent roll; the first to pass is thrown and the walk stops there, so at most one piece ever appears. When every entry fails the blast throws nothing; with the two entries the stock rules name, `GASTANK` and `PIECE`, that is a little over seven explosions in ten.

Order therefore decides how often each piece is seen rather than only which artwork is used. The first entry is thrown in fifteen explosions out of a hundred. The second is only offered its roll in the eighty-five where the first failed, and each entry after that in fewer again.

The walk is bounded by the list's own length, so an empty list costs the explosion its debris and nothing else.
