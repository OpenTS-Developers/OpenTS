---
key: NextParticleOffset
summary: How far a successor particle is placed from the one it replaces.
see_also: ["NextParticle", "Radius"]
when_omitted:
  kind: value
  value: 0,0,0
---

```ini title="rules.ini"
[MYGASSEED] ; a ParticleType registered in [Particles]
Image=gaslrgmk
BehavesLike=Gas
MaxEC=448
NextParticle=MYGASCLOUD ; a ParticleType registered in [Particles]
NextParticleOffset=0,0,150 ; the cloud forms 150 leptons above where the seed died
```

Three lepton components — X, Y and Z — added to the position an expiring particle held when its [`NextParticle`](/keys/nextparticle/) successor is created there. Only gas, weak gas and web systems apply it. A smoke system scatters its pair of successors with [`Radius`](/keys/radius/) and never reads the offset, and fire, spark and railgun systems create no successors at all.

The offset is applied once, at the moment of the replacement, and each link of a chain adds its own: three types that each raise their successor by 150 leptons leave the last of them 300 leptons above where the chain began.

:::note[A value with fewer than three components reads as the default]
`NextParticleOffset=0,0` is short of the three components an offset needs, so the key reads as its default and the debug log records the line; [INI syntax](/formats/ini-syntax/#malformed-values) has the rule.
:::
