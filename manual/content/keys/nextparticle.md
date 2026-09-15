---
key: NextParticle
summary: The particle type created in place of this one when it expires.
see_also: ["NextParticleOffset", "Radius", "MaxEC", "BehavesLike"]
when_omitted:
  kind: value
  value: none
---

```ini title="rules.ini"
[MYGASCLOUD] ; a ParticleType registered in [Particles]
Image=CLOUD1
BehavesLike=Gas
MaxEC=1000
NextParticle=MYGASTHIN ; a thinner cloud takes its place as it expires

[MYGASTHIN] ; a ParticleType registered in [Particles]
Image=CLOUD1D
BehavesLike=Gas
MaxEC=50
DeleteOnStateLimit=yes ; the chain ends here: no NextParticle
```

Whether a successor appears at all is decided by the system holding the particle, not by the particle. A gas, weak gas or web system replaces each expiring particle with one successor, placed at [`NextParticleOffset`](/keys/nextparticleoffset/) from where the original died. A smoke system replaces it with two, thrown out to either side by [`Radius`](/keys/radius/) and ignoring the offset. Fire, spark and railgun systems never create successors, so the setting is inert on any type they hold.

The successor inherits the speed its predecessor had reached and, in a gas, weak gas or web system, the sideways wander its behavior routine has built up, so a chain keeps its momentum rather than restarting. It does not inherit anything else: lifetime, damage, behavior and artwork all come from the successor's own section. `NextParticle=<none>` clears the setting; `NextParticle=none` does not. That name, like any other the game has not heard of, silently registers a new particle type with nothing configured in it, so its particles do nothing and vanish on their first frame.

:::danger[A Fire successor stops the game when it damages]
Successors are created outside the system that spawns them, and a `Fire` particle asks its system which object to spare when it applies damage. Naming a `Fire` particle as a successor therefore stops the game the first time one of those flames comes to apply damage with a live object in its cell; a flame given no [`Damage`](/keys/damage/#scope-particletype) never reaches that step. A `Fire` type is unaffected as the particle a system holds directly, and only exposed as the successor of another.
:::

:::danger[In a smoke system every link doubles the particle count]
Each expiring particle is replaced by two, so a chain of three types turns one particle into four, and a fourth link into eight. A chain that leads back to a type already in it never terminates: the count doubles every generation until the game runs out of memory. Gas, weak gas and web systems replace one with one, so a loop there holds the count steady but leaves a cloud that never finishes dying.
:::
