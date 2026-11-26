---
key: Engineer
scope: side
label: Side survivor engineer type
see_also: ["system:capture", Crew, Technician]
when_omitted:
  kind: inherited
  note: The `[General]` `Engineer` type.
---

```ini title="rules.ini"
[GDI] ; the side's section
Engineer=GDIENG ; a type registered in [InfantryTypes]
```

The InfantryType a structure of this side can pick on its engineer roll, in place of the `[General]` [`Engineer`](/keys/engineer/#scope-global-rules) type. The `[General]` scope gives the conditions for that roll and the limits on a sale.

The value applies to structures whose owning house [acts as](/keys/actslike/) a country on this side.

Writing `<none>` clears a type set earlier, by an earlier rules file, and returns the side to the `[General]` type.
