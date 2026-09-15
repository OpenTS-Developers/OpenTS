---
key: NamedCivilians
summary: Flag intended to name civilian objects outright.
no_effect: true
see_also: [Nominal]
when_omitted:
  kind: value
  value: "no"
---

The flag is stored under its own member on the rules object, and nothing reads that member back. The tooltip that names the object under the cursor, the sidebar, and the multiplayer synchronization checksum all ignore it.

Whether an object is named or glossed over with a generic label is decided per type instead, by [`Nominal`](/keys/nominal/). The tooltip reads it on the object's type and, for a structure, on the runtime instance as well. Without it an enemy object is shown only as an enemy soldier, vehicle or structure.
