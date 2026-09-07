---
key: CrewEscape
summary: The chance that a destroyed vehicle's crew steps out as an infantry survivor.
see_also: ["system:capture", Crew, Crewed, Passengers, "system:transports"]
when_omitted:
  kind: value
  value: ".5"
---

The value is a fraction, weighed against a random figure between 0 and 1 and passing only while it is the larger, so `0` never produces a crew. The roll is taken once and yields at most one soldier, whose type comes from [`Crew`](/keys/crew/) or [`Technician`](/keys/technician/). That soldier appears at the wreck with a strength between 5 and half its own maximum, hunting for a computer house and standing guard for a human one.

Only a vehicle reaches the roll, and only under **All of:**

- the hit that destroyed it was not flagged as leaving no crew — an energizing laser fence, a collapsing bridge and a crashing aircraft all set that flag;
- it is [`Crewed=yes`](/keys/crewed/);
- it declares no [`Passengers`](/keys/passengers/) at all.

A transport is therefore excluded by its capacity even when it is crewed and empty. A structure never consults the setting; its own count comes from [`SurvivorRate`](/keys/survivorrate/) and [`SurvivorDivisor`](/keys/survivordivisor/) instead.

A vehicle that was taken by a hijacker skips the roll entirely. [The hijacker steps back out](/systems/capture/#stealing-a-vehicle) in the crew's place, at any setting and without the `Crewed=yes` requirement.

:::caution[A death animation removes the crew altogether]
A vehicle whose artwork declares [`DeathFrames`](/keys/deathframes/) is not finished off by the hit that destroys it. It drops to one point of strength, plays the animation out, and is then exploded and deleted from its own update — a path that never reaches the roll. Such a vehicle produces no crew at any setting, and drops no truck crate either.
:::
