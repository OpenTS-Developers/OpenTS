---
key: DeadBodies
summary: The corpse animations left where an infantryman finishes a death sequence.
see_also: [InfDeath, Doggie, InfantryExplode, FlamingInfantry]
when_omitted:
  kind: value
  value: ""
---

```ini title="rules.ini"
[AudioVisual]
DeadBodies=MYDEATH_A,MYDEATH_B ; AnimTypes registered in [Animations]
```

An infantryman who plays a death sequence to its last frame has one entry created at his center and is then deleted. Two deaths reach it: the gun death a warhead with [`InfDeath=1`](/keys/infdeath/) causes and the explosion death `InfDeath=2` causes. The engine also lists a second explosion death and a grenade death, but nothing ever starts either one, and the fire death is taken only by a [`Doggie=yes`](/keys/doggie/) infantryman, who is deleted without a corpse. The entry is drawn from the whole list with equal weight.

Most deaths never get this far. A soldier who was already falling and comes down in water, a prone [`Cyborg=yes`](/keys/cyborg/) infantryman and a jumpjet infantryman are all deleted before a sequence is chosen at all, and several of the `InfDeath` cases delete the soldier outright as well, so the corpse list covers only the deaths that are animated where the soldier stood.

:::danger[An empty list crashes the game at the first gun or explosion death]
The entry is chosen by dividing a random figure by the number of entries and taking the remainder, which with an empty list is a division by zero. Set at least one entry before any warhead in the mod carries `InfDeath=1` or `InfDeath=2`.
:::

