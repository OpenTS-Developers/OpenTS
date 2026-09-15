---
key: Disableable
summary: Parsed flag that the engine never uses.
no_effect: true
see_also: ["system:emp-pulse", "system:power", Powered]
when_omitted:
  kind: value
  value: "yes"
---

Nothing reads the stored flag, so no type can be exempted from being shut down this way. What actually shuts a structure down is decided elsewhere and reads none of it. [An EM pulse](/systems/emp-pulse/#what-a-pulse-reaches) powers off and stuns every building it catches, sparing only a limpet mine, a core defender ([`IsCoreDefender=yes`](/keys/iscoredefender/#scope-buildingtype)), and a building whose type sets [`InvisibleInGame=yes`](/keys/invisibleingame/). A house short of power stops the functions of the structures that [`Powered=yes`](/keys/powered/) marks as needing it.
