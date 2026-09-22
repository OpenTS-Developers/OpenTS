---
key: XVelocity
summary: One horizontal axis of the spread of speeds a spark is thrown at.
see_also: ["YVelocity", "MinZVelocity", "ZVelocityRange", "BehavesLike"]
when_omitted:
  kind: value
  value: "1"
---

```ini title="rules.ini"
[MYSPARK] ; a ParticleType registered in [Particles]
BehavesLike=Spark
MaxEC=500
XVelocity=10 ; sparks are thrown under 10 leptons a frame along this axis, either way
YVelocity=10
MinZVelocity=40
ZVelocityRange=15
```

Every spark in a burst is thrown at its own random speed along this axis. The draw can be anything below this figure, in either direction, and it is measured in leptons a frame. The three axis settings do two jobs at once: together they fix how fast the spark travels, and they fix which way it goes. The type is thrown by its holding system, the [particle system](/systems/particle-systems/) whose `HoldsWhat=` names this type. That system adds a direction to the draw vector, either its own spawn direction or one random direction shared by the whole burst. The sum is normalized and scaled back to the length of the spark's own draw, so the added direction steers the spark without changing how fast it moves. Widening the spread allows longer draws, so a burst is thrown both faster and looser.

Only a spark system, a particle system whose [`BehavesLike=Spark`](/keys/behaveslike/#scope-particlesystemtype), reads the three axis settings. It reads them from the type it holds whatever that type's own [`BehavesLike`](/keys/behaveslike/#scope-particletype), so a type of another behavior held by a spark system is thrown along the drawn direction too. A type held by any other kind of system ignores them. A negative figure behaves as its magnitude does, since the spread is symmetrical either way.

:::danger[Zero stops the game]
The random speed is drawn by dividing by this figure, so `XVelocity=0` divides by zero and the game stops the moment a spark system holding the type fires its first burst. The same applies to [`YVelocity`](/keys/yvelocity/) and [`ZVelocityRange`](/keys/zvelocityrange/), each of which is divided by separately.
:::
