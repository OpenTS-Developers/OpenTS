---
key: SuspendDelay
summary: Minutes of game time a team stays suspended after a base attack emptied it.
see_also: ["system:base-attacked", SuspendPriority]
when_omitted:
  kind: value
  value: "2"
---

The team object is not destroyed when a base attack [suspends it](/systems/base-attacked/#teams-are-emptied-first). It is stripped of its members, and then does nothing at all until this countdown expires. The end of the countdown does not put the team back to work. A team that had once reached full strength, meaning the member count its TaskForce asks for, is deleted at that moment. A team that never reached it gets one pass at recruiting, and is then dissolved by the unfilled-team rules if it is still empty. That dissolving happens outside a campaign only; in a campaign the empty team stays and keeps trying to recruit. Every team the same attack suspends receives the same countdown, and a later attack while the countdown is still running restarts it from the top. A shorter countdown brings that ending forward: a team that never filled starts recruiting again sooner, and one that had filled is deleted sooner. [`SuspendPriority`](/keys/suspendpriority/) decides which teams are suspended in the first place.

```ini title="rules.ini"
[General]
SuspendDelay=.5  ; suspended teams sit out half a minute
```
