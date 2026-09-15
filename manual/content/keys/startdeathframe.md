---
key: StartDeathFrame
summary: The frame a shape-drawn vehicle's wreck animation begins at.
see_also: ["DeathFrames", "DeathFrameRate", "MaxDeathCounter", "Facings"]
when_omitted:
  kind: computed
  note: Facings × (FiringFrames + WalkFrames + 1), or -1 for a vehicle with no death frames.
---

The death run starts here, and unlike the walk, standing and firing blocks it is a single run shared by every facing rather than one run per facing.

The derived default leaves room for a walk block, a firing block and one further frame for each facing. That last frame stands in for the one standing frame a vehicle with [`FiringFrames`](/keys/firingframes/) is given by default. The derivation never reads [`StandingFrames`](/keys/standingframes/) itself, so a vehicle with a longer standing block has to name this frame outright.

:::caution[Naming this frame does not move the wreck's lifetime]
[`MaxDeathCounter`](/keys/maxdeathcounter/) defaults to the *derived* start frame plus [`DeathFrames`](/keys/deathframes/), and that default is settled before the section's own value for this key is read. Writing this key out therefore leaves the wreck lifetime where the derivation put it; artwork that moves the death run has to name both.
:::
