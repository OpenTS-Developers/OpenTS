---
key: Points
summary: Parsed score value that the engine never uses.
no_effect: true
see_also: [ThreatPosed, Category]
when_omitted:
  kind: value
  value: "0"
---

The name promises a score awarded for destroying an object of the type. The one assignment stores the figure twice, once under that name and once as the type's reward rating. Neither copy reaches anything that runs.

No gameplay path reads the score copy. The reward copy is added to [`ThreatPosed`](/keys/threatposed/) to give an object its worth as a prize. The only routine that ranks objects by that worth, a picker that would aim a superweapon at the most valuable enemy structure, has no caller.
