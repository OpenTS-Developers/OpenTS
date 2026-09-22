---
key: DefaultFirestormExplosionSystem
summary: The ParticleSystemType spawned in place of the usual explosion when the firestorm warhead destroys a vehicle or aircraft.
see_also: ["system:laser-fences"]
when_omitted:
  kind: value
  value: none
---

Between seven and nine of these systems are created at the victim's center, each told to send its sparks in a random direction. The branch is chosen by the killing warhead being exactly the one [`FirestormWarhead`](/keys/firestormwarhead/) names. It replaces the explosion the victim would otherwise have produced: an aircraft's [`Explosion`](/keys/explosion/) list, and a vehicle's ordinary explosion or water splash. A vehicle whose art sets [`DeathFrames`](/keys/deathframes/) above `0` throws no burst: it plays that death sequence instead and then explodes the ordinary way.
