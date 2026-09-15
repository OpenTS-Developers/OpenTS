---
key: GateUp
summary: The sound played at a gate as its door starts closing.
see_also: ["system:walls-and-gates", "Gate"]
when_omitted:
  kind: value
  value: none
---

The sound plays once, at the gate's own position, on the pass that starts the door closing, after [`GateCloseDelay`](/keys/gateclosedelay/) has expired with the footprint clear.

Despite the names, this is the closing sound and [`GateDown`](/keys/gatedown/) is the opening one.
