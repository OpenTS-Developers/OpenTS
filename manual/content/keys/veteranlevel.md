---
key: VeteranLevel
summary: The rank at which a reinforcement trigger creates this team's members.
see_also: ["system:veterancy"]
when_omitted:
  kind: value
  value: "1"
---

The rank applies only to members created by the [Reinforcement (team)](/mapping/actions/taction-reinforcements/) and [Reinforcement (team) at waypoint](/mapping/actions/taction-reinforcements-special/) trigger actions. Transports and their passengers take it alike. Members that an AI team recruits or has built keep the rank they already have.

| Value | Rank the member starts at |
| --- | --- |
| `0` | Below rookie, at `-0.25` experience |
| `1` | Rookie, as created |
| `2` | Veteran |
| `3` | Elite |

Any other value leaves the member at rookie, as `1` does.

[`VeteranCap`](/keys/veterancap/) limits the starting rank too. With the default cap, a team with `VeteranLevel=3` arrives veteran. A [`Trainable=yes`](/keys/trainable/) member above the cap [drops to it](/systems/veterancy/#the-experience-ceiling) the first time it destroys an object whose owner does not count it as an ally, even an object worth no experience.

`VeteranLevel=0` is the only team or rules setting that gives an object negative experience. A map's [placed-object records](/formats/scenario-objects/) can also give an object negative experience.

A below-rookie member fights exactly as a rookie does, because neither rank has abilities. It differs from a rookie in three ways:

- It draws a different [rank insignia](/systems/veterancy/#rank-display).
- A veterancy crate or an armory promotes it differently, as [promotion without kills](/systems/veterancy/#promotion-without-kills) describes.
- If it is `Trainable=yes`, it must earn `1.25` experience from kills to reach veteran, where a rookie needs `1`.
