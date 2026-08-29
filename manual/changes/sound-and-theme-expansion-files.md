---
title: Read the base and expansion sound and theme files together
category: feature
release: 0.2.0
targets:
- type: format
  id: sound-ini
  effect: changed
- type: format
  id: theme-ini
  effect: changed
breaking: true
migration:
- 'Check a deployment that ships both a base file and its expansion counterpart: both are now read.'
credit:
- ZivDero
---

Startup now reads `SOUND.INI` and `SOUND01.INI` into one database, and `THEME.INI` and
`THEME01.INI` into another. Whichever of a pair are there are read, the expansion's file
over the base one, so an entry both name comes from the expansion and an entry only the
base names is kept.

Either file of a pair on its own is now enough. A deployment that ships only the expansion's
sounds or themes starts, where before the base file had to be there as well; startup stops
only when neither file of a pair can be read.

Whether the expansion is installed no longer decides which file is read. `SOUND.INI` was
passed over entirely wherever Firestorm was installed, and `THEME01.INI` was passed over
wherever it was not.
