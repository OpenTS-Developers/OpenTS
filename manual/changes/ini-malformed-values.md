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

A floating-point number, point, offset, vector, color or rectangle the reader cannot convert now reads as the key's default. That is the value the key takes when the assignment is absent. The debug log records the file, section, key and value. A value short of three components used to stop the game while the rules were read, and so did a value that does not start with a number. A short color filled its missing channels from whatever was last in the storage it was scanned into.
