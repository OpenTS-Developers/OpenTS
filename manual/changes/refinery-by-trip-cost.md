---
title: Choose a refinery by what the trip really costs
category: feature
release: 0.2.0
targets:
- type: key
  id: Dock
  effect: changed
- type: system
  id: tiberium
  effect: changed
credit: [ZivDero, Rampastring]
---

A loaded harvester now weighs every refinery type in its [`Dock`](/keys/dock/) list together and goes to the nearest free bay; it used to take the first listed type that had one. When a busy refinery is so much nearer that waiting is quicker than driving to a free one, the harvester waits. The wait is estimated from the loads being unloaded and queued, so it follows a mod's [`Storage`](/keys/storage/), [`Speed`](/keys/speed/) and [`HarvesterDumpRate`](/keys/harvesterdumprate/).
