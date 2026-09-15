---
title: Fall back to the base side archive when the expansion's is absent
category: fix
release: 0.2.0
targets:
- type: format
  id: mix
  effect: changed
credit:
- ZivDero
---

A mission in an expansion campaign refused to start unless the installation had that
expansion's side CD archive, `E01SCD01.MIX` or `E01SCD02.MIX`. An installation holding
those members in `SIDECD01.MIX` and `SIDECD02.MIX` instead has every file the mission
asks for, but the game looked for the expansion archive alone and stopped with
`Unable to read scenario!` without it. The side's base archive is now mounted when the
expansion archive is absent.
