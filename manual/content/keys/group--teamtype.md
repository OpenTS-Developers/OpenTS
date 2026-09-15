---
key: Group
scope: teamtype
label: Team recruitment group
see_also: ["system:ai-team-production", Recruiter]
when_omitted:
  kind: value
  value: "-1"
  note: A TeamType left at -1 takes the group of its TaskForce, which is itself -1 unless that section sets one.
---

A team [recruits](/systems/ai-team-production/#recruitment) an object whose own group matches this one. It stamps this group onto every member as it joins, so a group is what one team hands to the next. The value `-2` is the wildcard that matches any object, and [`Recruiter=yes`](/keys/recruiter/) reaches the same result for one TeamType without changing what its members are stamped with. Objects start out on `-1`, so a TeamType that leaves both this key and its TaskForce's alone recruits freely from anything it has not already claimed.
