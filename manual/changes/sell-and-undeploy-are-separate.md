---
title: A sale no longer undeploys a structure
category: fix
release: 0.3.0
targets:
- type: key
  id: UndeploysInto
  effect: changed
- type: action
  id: TACTION_SELL_ATTACHED
  effect: changed
credit:
- JoyfulShush
---

Selling a structure that names an `UndeploysInto=` vehicle now pays its refund and takes it off the map; the structure no longer turns into the vehicle. It releases survivors like any other sold structure, so selling a construction yard leaves its crew and plays the "structure sold" announcement. Only an undeploy order packs such a structure up, and an order that cannot undeploy it now leaves it in place instead of selling it.
