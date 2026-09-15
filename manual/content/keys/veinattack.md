---
key: VeinAttack
summary: AnimType a vein cell attaches to itself to hurt what stands in it.
see_also: ["system:veins", "IsVeins", "VeinDamage", "VeinholeWarhead"]
when_omitted:
  kind: value
  value: none
---

```ini title="rules.ini"
[AudioVisual]
VeinAttack=VEINATAC
```

The animation is the whole of the [vein attack](/systems/veins/#standing-in-veins). A cell whose vein has finished growing and lies flat on level ground creates one over itself as soon as a vulnerable object is in it; growing vein, vein on a slope, and the veinhole itself never qualify. A vulnerable object is a building, vehicle, infantryman or aircraft standing within 5 leptons of the ground that has neither [`ImmuneToVeins=yes`](/keys/immunetoveins/) nor the [`VEIN_PROOF`](/systems/veterancy/#abilities) veteran ability. The animation applies the damage on every other frame until it removes itself. A cell takes no new attack while one is already attached to it. A single trigger creates one animation for every vulnerable object standing there at that moment, and each of them deals the full damage. A cell holding three vulnerable infantry therefore damages everything in it three times over.

:::danger[An unresolved setting faults the first time veins ripen]
This setting has no built-in value, and the cell creates the animation without checking what it got. With the setting unresolved, the first time a vulnerable object and mature flat vein meet in the same cell, the cell builds an animation from a null type and faults. That holds whether the object arrives or the vein ripens under it. The named animation must also set [`IsVeins=yes`](/keys/isveins/#scope-animtype) in `art.ini`, or it plays as ordinary artwork and permanently blocks its cell from attacking again.
:::
