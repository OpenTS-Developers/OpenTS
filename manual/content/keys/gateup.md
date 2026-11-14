---
key: GateUp
summary: The default sound played at a gate as its door starts going up.
see_also: ["system:walls-and-gates", "Gate"]
when_omitted:
  kind: value
  value: none
---

The sound plays once, at the gate's position, when its door starts going up. That happens after [`GateCloseDelay`](/keys/gateclosedelay/) has run out with nothing standing in the gate. 

This sound is only played if the gate did not set [`GateUpSound`](/keys/gateupsound/). If it did, it would play its specific sound instead.
