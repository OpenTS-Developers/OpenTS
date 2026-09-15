---
key: BuildDefense
summary: Parsed defensive structure list for a computer base that the engine never uses.
no_effect: true
see_also: ["system:ai-base-building", BuildPDefense, IsBaseDefense]
when_omitted:
  kind: value
  value: ""
---

A computer house does not pick its defenses from a list. Candidates are gathered at the moment a defense node is filled in, from every type it [may own](/keys/owner/) that sets a value above zero in the category [the defense planner](/systems/ai-base-building/#base-defenses) has chosen. Those values are computed from each type's own weapon. Naming a type here neither adds it to that pool nor keeps it out.
