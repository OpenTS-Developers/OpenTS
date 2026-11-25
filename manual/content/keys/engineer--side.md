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

The InfantryType a structure of this side can leave as an engineer survivor, in place of the `[General]` [`Engineer`](/keys/engineer/#scope-global-rules) type. The `[General]` scope gives the roll that picks it, the two limits on a sale, and the rule that a structure which has ever changed hands never produces it; all three apply to this value too.

The value applies to structures whose owning house [acts as](/keys/actslike/) a country on this side. It does not change which structures take the roll: only a structure whose type builds structures produces this type.

Writing `<none>` clears a type set earlier, by an earlier rules file, and returns the side to the `[General]` type.
