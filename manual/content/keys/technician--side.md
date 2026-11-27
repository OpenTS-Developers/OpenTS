---
key: Technician
scope: side
label: Side technician type
see_also: ["system:capture", Crew]
when_omitted:
  kind: inherited
  note: The `[General]` `Technician` type.
---

```ini title="rules.ini"
[Nod]
Technician=NODTECH ; a type registered in [InfantryTypes]
```

The InfantryType a structure or vehicle of this side leaves as its technician, in place of the `[General]` [`Technician`](/keys/technician/#scope-global-rules) type. [Survivor types](/systems/capture/#survivor-types) gives the two cases that pick one.

The value applies to every structure and vehicle whose owning house [acts as](/keys/actslike/) a country on this side, and it applies even when that house's own country has no side. That second case is the one that makes every survivor a technician.

Writing `<none>` clears a type set earlier, by an earlier rules file, and returns the side to the `[General]` type.
