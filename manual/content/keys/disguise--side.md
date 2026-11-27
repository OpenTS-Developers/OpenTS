---
key: Disguise
scope: side
label: Side disguise type
see_also: [Disguised, Nominal]
when_omitted:
  kind: inherited
  note: The `[General]` `Disguise` type.
---

```ini title="rules.ini"
[Nod] ; the side's section
Disguise=NODSPY ; a type registered in [InfantryTypes]
```

The InfantryType whose name and artwork a disguised soldier of this side shows, in place of the `[General]` [`Disguise`](/keys/disguise/#scope-global-rules) type.

The value applies to every disguised soldier whose owning house [acts as](/keys/actslike/) a country on this side. The `[General]` scope gives the ownership tests for the name and the artwork, and they apply to this value too.

Writing `<none>` clears a type set earlier, by an earlier rules file, and returns the side to the `[General]` type.
