---
title: Set how much damage a Tiberium type deals to infantry
category: feature
release: 0.3.0
targets:
- type: key
  id: DamageToInfantry
  effect: added
credit: 
- JoyfulShush
- ZivDero
---

In a Tiberium type's section in `rules.ini`, `DamageToInfantry` sets the damage its cells deal to infantry that finish moving into them. A type that leaves the key unset keeps dealing a tenth of its `Power`, so rules files that do not set it behave as before.

Credits to ZivDero for their original Vinifera implementation for this feature.
