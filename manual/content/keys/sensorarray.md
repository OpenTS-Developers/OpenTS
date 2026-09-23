---
key: SensorArray
summary: Whether the structure marks the cells around it as sensed, revealing cloaked objects standing there.
see_also: ["system:power", "system:cloaking"]
when_omitted:
  kind: value
  value: "no"
---

The array marks every cell within [`CloakRadiusInCells`](/keys/cloakradiusincells/) as sensed for its house when the structure first opens, if it is operational at that moment. An array that is not operational when it opens marks nothing, and its power returning does not mark the cells either. Its cells are marked the next time any house's cloak field finishes growing while the array is operational, so on a map without cloak generators it never marks them.

Coverage is lifted when the structure is taken off the map. A cell that another array of the same house also covers stays sensed. Capturing the array moves its coverage: the old owner stops sensing the cells, and the new owner senses them at once if the array is operational.

Marking a cell reveals a cloaked object standing on it to the array's house, and a fully faded enemy structure on a marked cell is made visible again as a shadowy outline.

:::caution[A shortfall does not lift sensor coverage]
Unlike a [cloak generator](/keys/cloakgenerator/), whose field collapses the moment the base runs short of power, an array keeps every cell it has marked until it is taken off the map or captured. Low power matters only to an array that has not marked its cells yet: the marking that runs when a cloak field finishes growing skips an array that is switched off, stunned or short of power.
:::
