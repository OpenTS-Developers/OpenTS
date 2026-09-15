---
key: RequiredAddon
summary: The expansion a campaign belongs to, which decides when the mission list offers it.
see_also: ["Description", "Scenario"]
when_omitted:
  kind: value
  value: "0"
---

`0` marks the campaign as part of the base game and `1` as part of Firestorm. The mission list offers base game campaigns only while no expansion is running, and an expansion's campaigns only while that same expansion is running. The two sets never appear side by side. A campaign whose number names an expansion that is not the running one is left out of the list entirely.
