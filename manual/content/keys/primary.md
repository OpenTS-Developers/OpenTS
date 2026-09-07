---
key: Primary
summary: The WeaponType in the object type's first weapon slot.
see_also: ["system:target-selection", "system:veterancy"]
when_omitted:
  kind: value
  value: none
---

The first slot is the one every other system falls back on: it decides whether the type counts as armed at all, supplies the projectile the target scan tests for [`AG`](/keys/ag/), and is the slot a tie in weapon choice resolves to.

```ini title="rules.ini"
[MYTANK] ; example UnitType
Primary=MyCannon ; names the [MyCannon] weapon section
```

Writing `none` or `<none>` empties the slot. A type with an empty first slot is unarmed throughout targeting: it never retaliates, a human house's objects outside a team will not automatically fire on it while it is a building, and it accepts candidates within [`GuardRange`](/keys/guardrange/) instead of running a range test.

An ID with no section of its own is not an error — it registers a WeaponType under that name carrying only its defaults.

The slot's contents are substituted at runtime. An elite object fires its [`Elite`](/keys/elite/) weapon here, and a building resolves the slot through its plugged-in upgrades before either; [the elite weapon](/systems/veterancy/#the-elite-weapon) covers both.

:::danger[This assignment can change weapon numbering]
A weapon the rules [`[Weapons]` list](/formats/rules-registries/) leaves out is numbered where this key first names it, so adding, removing or renaming an unlisted weapon here shifts the weapon numbers stored in [Do Explosion At](/mapping/actions/taction-do-explosion/) trigger actions. Listing the weapon settles its number instead.
:::
