---
title: Declare sides in the side list alone
category: fix
release: 0.2.0
targets:
- type: key
  id: Side
  scope: housetype
  effect: changed
- type: key
  id: Side
  scope: themes
  effect: changed
- type: key
  id: SpeechSide
  effect: changed
- type: format
  id: rules-registries
  effect: changed
breaking: true
migration:
- List every side under `[Sides]` with its countries written out. A country's `Side=` places it only while no `[Sides]` entry does, and a side name inside a `[Sides]` value is no longer expanded into that side's countries.
credit: [ZivDero]
---

`[Sides]` is now the roster of record. A country it places keeps that side whatever its own
`Side=` says, with the conflict logged, so the raw Tiberian Sun rules' `[Nod] Side=GDI` no longer
moves Nod onto GDI's side. A `Side=` naming a side that `[Sides]` never declared, on a country, a
score or a mission's `SpeechSide=`, is logged and ignored instead of creating a side, and a
`[Sides]` value names countries alone: a side name inside one no longer splices that side's roster
in.
