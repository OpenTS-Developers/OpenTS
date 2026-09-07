---
key: Warhead
scope: weapontype
label: Weapon warhead
see_also: ["Verses", "Damage", "Projectile", "Bright", "system:target-selection"]
when_omitted:
  kind: value
  value: none
---

The warhead is attached to every shot and decides what the [`Damage=`](/keys/damage/#scope-weapontype) figure does where it lands: how much of it survives the target's armor, how far it spreads, which explosion is drawn, and whether it pulses, webs, burns, knocks down walls or clears Tiberium.

```ini title="rules.ini"
[MyCannon] ; example WeaponType
Damage=90
Warhead=AP ; a WarheadType registered in [Warheads]
Projectile=Cannon ; a BulletType, registered by a weapon naming it as its Projectile
```

The same warhead is what the beam of an [`IsRailgun=yes`](/keys/israilgun/) weapon and the wave of an [`IsSonic=yes`](/keys/issonic/) weapon carry, and what the [Do Explosion At](/mapping/actions/taction-do-explosion/) trigger action detonates — that action takes a weapon's damage and warhead and nothing else, so the projectile, the sounds and the firing animation play no part in it.

Its [`Verses`](/keys/verses/) table reaches well past the moment of impact. It is both effectiveness terms of the [threat score](/systems/target-selection/#the-threat-score), the test that decides whether an object bothers answering fire, and — through the `heavy` entry standing at exactly nothing — what confines a dog to infantry targets.

A name the game does not already know is registered as a new warhead of that name rather than rejected. Such a warhead does full damage against every armor class, spreads over a single cell and draws no explosion at all, so a misspelling produces a shot that lands invisibly rather than an error.

:::danger[A weapon with no warhead crashes the game]
`Warhead=none`, and leaving the key unwritten, both leave the shot with nothing to detonate, and the detonation path asks the warhead what kind it is before checking that there is one, so the game stops the moment such a shot lands. A weapon in the first slot of an [`IsBaseDefense=yes`](/keys/isbasedefense/) structure never gets that far: its warhead is read while the rules are still being loaded, and the game stops there, before the scenario opens. In any other structure's first slot the warhead is read again whenever the owning house places, loses or captures a building — a reading skipped only while that house holds a single structure costing under 1000 — so in practice the game stops at the first such event.
:::
