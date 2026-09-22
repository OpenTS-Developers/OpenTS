---
key: Disableable
summary: Parsed flag that the engine never uses.
no_effect: true
see_also: ["system:emp-pulse", "system:power", Powered]
when_omitted:
  kind: value
  value: "yes"
---

Nothing reads the flag, so it cannot exempt a structure from being shut down. [An EM pulse](/systems/emp-pulse/#what-a-pulse-reaches) powers off and stuns every structure it catches except a limpet mine, which it destroys, and a type with [`ImmuneToEMP=yes`](/keys/immunetoemp/) or [`InvisibleInGame=yes`](/keys/invisibleingame/). Low power shuts down only structures whose type sets [`Powered=yes`](/keys/powered/) and drains power.
