---
key: CloakGenerator
summary: Whether the structure projects a cloaking field over the cells around it.
see_also: ["system:power"]
when_omitted:
  kind: value
  value: "no"
---

The field reaches [`CloakRadiusInCells`](/keys/cloakradiusincells/) and is not thrown up at once: the radius grows by one cell per game frame until it is complete, and collapses the same way. Every cell inside it is marked as cloaked for the owning house, and the mobile objects standing in each cell are asked to cloak as it passes over them.

The field falls whenever the structure stops being operational, which it does under **Any of:**

- it is switched off;
- it is stunned;
- its strength has reached zero;
- **All of:** its type is [`Powered=yes`](/keys/powered/), its type sets drain of its own, its type is [`TogglePower=yes`](/keys/togglepower/), and its house's power fraction is below 1.

The field regrows ring by ring once the structure is operational again. One caught mid-collapse resumes from the ring it had reached, and only a fully collapsed one starts over from the structure. A generator left at `Powered=no` keeps its field through a shortfall.

The frame a field finishes growing, every operational [`SensorArray=yes`](/keys/sensorarray/) structure on the map (of any house) re-stamps its coverage, refreshing the display of the objects the new field just faded wherever a sensor reaches them.
