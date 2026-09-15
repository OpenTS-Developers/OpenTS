---
key: Translucency
scope: particletype
label: Particle fade
see_also: ["Translucent25State", "Translucent50State", "BehavesLike"]
when_omitted:
  kind: value
  value: "0"
---

Only three levels of translucency can be drawn, and the value has to land on one of them: 25 draws the particle a quarter faded, 50 a half, and 75 through 127 three quarters. Every other number draws the particle solid, `0` among them. The particle holds the level in one signed byte, so the figure wraps modulo 256: 128 through 255 land below zero and draw solid too, while 281 lands on `25` and 306 on `50` again. The whole test is skipped below the High detail setting: at Medium detail particles are drawn solid whatever this says, and at Low detail smoke and spark particles are not drawn at all.

The figure is only where a particle starts. A [`Fire`](/keys/behaveslike/#scope-particletype) particle raises its own as its sequence passes [`Translucent25State`](/keys/translucent25state/) and [`Translucent50State`](/keys/translucent50state/). A smoke system adds a step to five of every six successors it creates, and to the particles it emits once the emitting system has aged past its [`SpawnTranslucencyCutoff`](/keys/spawntranslucencycutoff/). A puff authored at `25` is therefore usually drawn at `50`, and its successors more faded still.

`Spark` and `Railgun` particles are plotted as single pixels rather than drawn from artwork, and that path never reads the setting.

```ini title="rules.ini"
[MYSMOKEPUFF] ; a ParticleType registered in [Particles]
Image=SGRYSMK1
BehavesLike=Smoke
Translucency=25 ; a quarter faded when created, half once the plume thins
MaxEC=80
EndStateAI=20
```
