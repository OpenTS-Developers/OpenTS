---
key: UseMinDefenseRule
summary: Whether a house must hold its minimum number of defensive teams before any other AI trigger may spring.
see_also: ["system:ai-team-production", MinimumAIDefensiveTeams, IsBaseDefense]
when_omitted:
  kind: value
  value: "yes"
---

A defensive team is one whose type sets [`IsBaseDefense=yes`](/keys/isbasedefense/). With the rule off, [`MinimumAIDefensiveTeams`](/keys/minimumaidefensiveteams/) is never read, and a house may spring an offensive trigger while holding none of them. The rule is not the only path to that restriction: a house with no enemy is [held to defensive triggers](/systems/ai-team-production/#defensive-teams-and-the-enemy) whatever this value says.
