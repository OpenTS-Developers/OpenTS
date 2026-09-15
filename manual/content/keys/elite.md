---
key: Elite
summary: The WeaponType an elite object fires in place of its primary.
see_also: ["system:veterancy", "system:target-selection"]
when_omitted:
  kind: value
  value: none
  note: An object type with no elite weapon serves up its primary instead, so an elite object of that type fires exactly what it always did.
---

Once an object reaches elite rank, every request for its primary weapon resolves to this one instead: target selection, range tests, reload delay, and the shot itself. The secondary weapon is never substituted, and a veteran fires its ordinary weapons. [What a rank changes](/systems/veterancy/#the-elite-weapon) covers the rest of the substitution.

```ini title="rules.ini"
[MYTANK] ; example UnitType
Primary=MyCannon     ; each names its own weapon section
Elite=MyEliteCannon
```

The elite slot takes its muzzle geometry from the primary's [`PrimaryFireFLH`](/keys/primaryfireflh/), [`PBarrelLength`](/keys/pbarrellength/), and [`PBarrelThickness`](/keys/pbarrelthickness/) art settings, so the replacement fires from the same point on the artwork as the weapon it stands in for.

:::caution[An upgrade plug outranks this weapon]
A building resolves its weapon through the upgrades plugged into it before falling back to its own type. When a plug supplies a weapon for the requested slot, that weapon is used and this one never applies, whatever the building's rank.
:::
