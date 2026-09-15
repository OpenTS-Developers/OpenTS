---
key: IsBaseDefense
scope: teamtype
label: Defensive team
when_omitted:
  kind: value
  value: "no"
---

Every live team built from such a TeamType raises its house's defensive team count. The count is weighed against [`MinimumAIDefensiveTeams`](/keys/minimumaidefensiveteams/) while [`UseMinDefenseRule`](/keys/usemindefenserule/) is on before an AI trigger may spring. The team creation pass counts a house's live defensive teams again for itself and caps them against [`MaximumAIDefensiveTeams`](/keys/maximumaidefensiveteams/). An AI trigger whose first TeamType is defensive, and whose second is defensive or absent, is itself handled as a defensive trigger. A member of such a team is also [called up to defend the base](/systems/base-attacked/#which-objects-qualify) when it comes under attack, where a member of any other team is passed over.
