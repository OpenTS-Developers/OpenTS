---
key: Crawls
summary: Chooses whether a prone soldier of this type crawls at two thirds of its speed or moves at half again its speed.
see_also: ["Sequence", "Speed"]
when_omitted:
  kind: value
  value: "yes"
---

The flag decides one thing: how fast a soldier of the type moves while it is prone. Set, prone movement is the type's speed less a third of it. Clear, prone movement is the type's speed plus half of it, so a soldier that ducks rather than crawls covers ground faster lying down than it does upright.

It does not choose an animation. A prone soldier that moves is put into the `Crawl` run of its [`Sequence`](/keys/sequence/) section at either setting; the flag only sets the pace.
