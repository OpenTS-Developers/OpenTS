---
key: Crew
scope: side
label: Side crew type
see_also: ["system:capture", CrewEscape, Technician, Engineer]
when_omitted:
  kind: inherited
  note: The `[General]` `Crew` type.
---

```ini title="rules.ini"
[GDI] ; the side's section
Crew=E1 ; the stock Light Infantry
```

The InfantryType a structure or vehicle of this side leaves as its ordinary survivor, in place of the `[General]` [`Crew`](/keys/crew/#scope-global-rules) type. [Survivor types](/systems/capture/#survivor-types) gives the whole choice, including the engineer and technician a survivor can be instead.

The value applies to every structure and vehicle whose owning house [acts as](/keys/actslike/) a country on this side. A vehicle leaves it only when its own [`CrewEscape`](/keys/crewescape/) roll succeeds.

Writing `<none>` clears a type set earlier, by an earlier rules file, and returns the side to the `[General]` type. The value also need not be listed under `[InfantryTypes]`; the `[General]` scope covers what an unlisted name gives.
