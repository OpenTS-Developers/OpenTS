---
key: InfantryExplode
summary: The explosion that stands in for an infantry death animation.
see_also: [InfDeath, Cyborg, DeadBodies, FlamingInfantry]
when_omitted:
  kind: value
  value: none
---

```ini title="rules.ini"
[AudioVisual]
InfantryExplode=MYINFBANG ; an AnimType registered in [Animations]
```

The animation is created at the soldier's own position, and the soldier is deleted rather than being left to play a death sequence, so no corpse from [`DeadBodies`](/keys/deadbodies/) follows it.

Four deaths reach it, and one non-death:

- a warhead with [`InfDeath=3`](/keys/infdeath/), which is the plain choice a mod makes for an exploding death;
- a [`Cyborg=yes`](/keys/cyborg/) infantryman killed while prone;
- a jumpjet infantryman, whatever killed it;
- a forced kill of a `Cyborg=yes` infantryman that was already falling at the time;
- and a `Cyborg=yes` infantryman brought to zero strength while upright by damage that was not forced. That soldier is not killed at all: he is set back to a quarter of his maximum strength, dropped prone and put into a crawl, and this explosion marks the moment his legs go.

Because the forced-kill case is a separate test rather than part of the chain, a falling cyborg that is forced to die while prone and not standing low over water creates the animation twice on the same frame.

:::danger[An unset animation crashes the game at the first cyborg or jumpjet death]
The explosion is created without first checking that an animation was named. Even a mod that never sets `InfDeath=3` reaches it through the cyborg and jumpjet paths, so leaving the key unset crashes the game the first time either dies.
:::
