---
key: AttachedParticleSystem
summary: The particle system a flame, spark or railgun weapon spawns as it fires.
see_also: ["UseFireParticles", "UseSparkParticles", "IsRailgun"]
when_omitted:
  kind: value
  value: none
---

Three flags reach for it, and nothing else does: [`UseFireParticles=yes`](/keys/usefireparticles/), [`UseSparkParticles=yes`](/keys/usesparkparticles/) and [`IsRailgun=yes`](/keys/israilgun/). Each spawns one system of the named type as the shot leaves, and each keeps its own hold on the object. A weapon that sets two of the flags spawns the same type twice over. Until that system has burned itself out the weapon is refused another shot, which is what ties the effect's own lifetime to the weapon's rate of fire. The refusal sits on top of the reload rather than replacing it: every shot pays its full [`ROF`](/keys/rof/#scope-weapontype), and a system still burning when the reload ends stretches the wait beyond that.

```ini title="rules.ini"
[MyRailgun] ; example WeaponType
IsRailgun=yes
AttachedParticleSystem=LargeRailgunSys ; a ParticleSystemType registered in [ParticleSystems]
AmbientDamage=200
Damage=0
```

A name the game does not already know is registered as a new particle system of that name rather than rejected. A misspelling therefore produces a system with none of the type's settings filled in instead of an error. Only `<none>` and an empty value resolve to nothing, and both leave whatever an earlier rules file set in place rather than clearing it.

:::caution[The value is cut off after nineteen characters]
The name is read into a twenty-character buffer, so a longer one is truncated before it is looked up. The truncated text is then registered as a particle system of its own, and the weapon spawns that empty system instead of the one intended.
:::

:::danger[A flame, spark or railgun weapon with no system named crashes the game]
None of the three flags checks that a system was named before spawning one. The first time such a weapon fires, the engine reads the settings of a type that is not there and the game stops. Give every weapon with one of those flags an `AttachedParticleSystem=`.
:::
