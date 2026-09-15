---
key: Helipad
summary: Lets a BuildingType accept an aircraft as a docking target and be approached at its docking cell rather than its center.
see_also: ["system:ai-base-building"]
when_omitted:
  kind: value
  value: "no"
---

The building answers a load request from an aircraft and refuses one from anything else. A player may order an aircraft into a pad only while the pad is free: it is not already linked to another aircraft and has no object attached to it. Docking is all the flag itself provides. A plain pad neither repairs a docked aircraft nor restores its ammo; that comes from the separate [`UnitRepair=`](/keys/unitrepair/) and [`UnitReload=`](/keys/unitreload/) flags, which the stock pads also set. An empty carryall ordered into one lands at ground level. While [the base plan is assembled](/systems/ai-base-building/#building-the-plan), a type with this flag is appended one to three extra times, so a computer house plans several of them wherever it plans one.
