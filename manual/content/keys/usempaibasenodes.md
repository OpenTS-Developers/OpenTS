---
key: UseMPAIBaseNodes
summary: Whether the computer houses of a skirmish or multiplayer game follow the base plans the map wrote for their start positions.
see_also: [NodeCount, PercentBuilt, Allies, "system:ai-base-building"]
when_omitted:
  kind: value
  value: "no"
---

Set to `yes` in the map's `[Basic]` section, the house holding start position N reads its base node list from the map section `[Spawn<N+1>]`. Waypoint `0` reads `[Spawn1]` and waypoint `7` reads `[Spawn8]`. These sections set the same [`NodeCount`](/keys/nodecount/), `000` onward, and [`PercentBuilt`](/keys/percentbuilt/) keys a campaign house record uses. The list is read for every house that holds a position, including a human player's, so a house the computer takes over later follows it too. A campaign game ignores the key.

A computer house that follows a map plan then builds as a campaign house does, and [AI base planning and building](/systems/ai-base-building/) owns the details. A node's cell is used as written instead of having to touch ground the house already holds. No power plant is inserted ahead of a node the house cannot power. The money-raising and fire-sale interventions are off. A destroyed base defense is rebuilt as the same type on the same cell.

A held position whose section has no node list leaves that house's list empty. The house then generates its own plan when its construction vehicle deploys, and it still builds under the rules above. A section for a position nobody holds, and a house started on open ground because the placed waypoints ran out, read nothing.

```ini title="multiplayer map file"
[Basic]
UseMPAIBaseNodes=yes

[Spawn2] ; whoever starts at waypoint 1
NodeCount=3
000=GACNST,54,71 ; construction yard BuildingType on the start position
001=GAPOWR,57,70 ; power plant BuildingType
002=GAPILE,0,0   ; barracks BuildingType; the cell is picked at build time
```
