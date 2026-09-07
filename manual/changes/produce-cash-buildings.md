---
title: Let a structure produce cash on an interval
category: feature
release: 0.2.0
targets:
- type: key
  id: ProduceCashStartup
  effect: added
- type: key
  id: ProduceCashStartupOneTime
  effect: added
- type: key
  id: ProduceCashAmount
  effect: added
- type: key
  id: ProduceCashDelay
  effect: added
- type: key
  id: ProduceCashBudget
  effect: added
- type: key
  id: ProduceCashResetOnCapture
  effect: added
- type: system
  id: produce-cash
  effect: added
- type: format
  id: save-games
  effect: changed
credit: [ZivDero, CCHyper, Rampastring]
---

Six BuildingType keys let a structure move money on its own. `ProduceCashAmount=` pays its owner that many credits every `ProduceCashDelay=` frames, and takes them instead when the figure is negative. `ProduceCashBudget=` caps the total one structure will ever move, and `ProduceCashResetOnCapture=yes` hands each new owner a fresh cap. `ProduceCashStartup=` pays a bonus to whoever captures the structure off a house that takes no part in the contest, once for good under `ProduceCashStartupOneTime=yes`.

Together they cover the oil derrick, the tech structure worth a lump sum on capture, and a structure that costs its owner to hold. A `Powered=yes` one stops while it is switched off, stunned, or short of power, and resumes its interval where it stopped.
