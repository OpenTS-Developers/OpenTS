---
title: Settle a repeated INI section or key instead of stopping on it
category: fix
release: 0.2.0
targets:
- type: format
  id: ini-syntax
  effect: changed
credit: [ZivDero]
---

A file that names the same section twice now continues the section it already opened, and an
assignment repeating a key takes the later value and moves the key to the end of its section,
which is what merging a later file already did. The two blocks of a repeated section used to be
kept as rival copies, which left the keys of one of them unreachable: the stock `nod10a` mission
carries `[HMEC]` twice and lost one of its two keys. A debug build stopped on an assertion at
either kind of repeat. A repeat within one file is now written to the debug log with the file,
section and key. Where a repeated key sat inside a numbered list such as `[VehicleTypes]`, the
entries after it move up by one, since only one copy remains.
