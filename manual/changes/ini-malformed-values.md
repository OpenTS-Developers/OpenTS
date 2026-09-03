---
title: Read a malformed number as its default instead of keeping garbage
category: fix
release: 0.2.0
targets:
- type: format
  id: ini-syntax
  effect: changed
credit: [ZivDero]
---

A floating-point number, point, offset, vector, color or rectangle whose value does not hold
the numbers it needs now reads as the key's default, and the debug log records the file,
section, key and value. A value short of three components used to stop the game while the
rules were read, a short color used to fill its missing channels from whatever was last in
the storage it was scanned into, and a value that was not a number at all used to do the same.
