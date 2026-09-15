---
key: Recruiter
summary: Whether a team of this type may recruit an object whose group does not match its own.
see_also: ["system:ai-team-production", Group]
when_omitted:
  kind: value
  value: "no"
---

Without it a team considers only objects with its own [group](/systems/ai-team-production/#recruitment), unless that group is `-2`, which matches everything; a team on any other group that no object shares recruits nobody. Setting it widens the scan to every object of the right kind. The 50-cell distance penalty a candidate outside the group is ranked with still applies, so a candidate that does match the group is preferred while one stands within that margin.
