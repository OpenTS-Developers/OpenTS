---
command_id: SelectOneLess
---

Deselects one object and leaves the rest selected. It removes the most recently selected object that is not kept at the front of the selection order. When every selected object is kept at the front, it removes the one that was selected first. If the view is following the removed object, it stops following.

An object is kept at the front when its type's primary weapon has a positive [`Damage`](/keys/damage/#scope-weapontype), but only if the type's section appears again in a rules file read after `RULES.INI`. [Multiplayer rules](/formats/multiplayer-rules/#when-they-are-read) lists the order the files are read in. The `Damage` that counts is the value set before that later file is read. A type whose section appears only in `RULES.INI` is never kept at the front, whatever its weapon.
