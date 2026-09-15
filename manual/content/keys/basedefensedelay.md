---
key: BaseDefenseDelay
summary: Minutes an attacker is ignored after it has already drawn a full base defense response.
see_also: ["system:base-attacked", ComputerBaseDefenseResponse]
when_omitted:
  kind: value
  value: ".25"
---

The countdown is stored on the attacking object rather than on the house that answered. While that attacker's countdown is still running, [the call-up](/systems/base-attacked/#when-the-call-up-is-refused) refuses outright. One satisfied response therefore stops every other house, not just the one that was hit, from answering that same attacker until it expires.

The countdown is set only when the ratings of the dispatched defenders, or of defenders already fighting the attacker, pushed the budget below zero. A budget cancelled to exactly zero leaves no countdown, and neither does an attacker whose own rating is zero. A response that ran out of qualifying candidates before covering its budget leaves no countdown at all. Such a response runs again on the attacker's next hit.
