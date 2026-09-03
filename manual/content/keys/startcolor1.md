---
key: StartColor1
summary: One end of the color range a spark or railgun particle is created at.
see_also: ["StartColor2", "ColorList", "ColorSpeed"]
when_omitted:
  kind: value
  value: 0,0,0
---

```ini title="rules.ini"
[MYWELDSPARK] ; a ParticleType registered in [Particles]
BehavesLike=Spark
MaxEC=500
XVelocity=16
YVelocity=16
ZVelocityRange=15
ColorList=(0,128,255),(255,255,255),(80,80,80),(0,0,0)
StartColor1=80,255,255 ; each spark is created somewhere between these two
StartColor2=255,255,100
ColorSpeed=.13
```

Written as a plain `red,green,blue` triplet without brackets, unlike the entries of [`ColorList`](/keys/colorlist/). Each new particle picks a color at a random point between this one and [`StartColor2`](/keys/startcolor2/), interpolating each component separately, so a burst is created as a spread of shades rather than one flat color. Components are held in one byte each and wrap above 255.

The picked color is where the [`ColorList`](/keys/colorlist/) blend starts: it stands in for the list's first entry while the particle is still on its first pair, and once the blend steps past that pair it is never used again. Leaving both ends black hands the job to the list's first entry instead, which is what most of the stock spark and railgun types do. Only [`Spark` and `Railgun`](/keys/behaveslike/#scope-particletype) particles are drawn as the lit pixel that uses the picked color; on every other behavior the pair is read and then never consulted.

:::note[A partial triplet reads as the default]
`StartColor1=80` names one channel where three are needed, so the particles start at their default color and the debug log records the line; [INI syntax](/formats/ini-syntax/#malformed-values) has the rule.
:::
