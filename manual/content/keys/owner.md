---
key: Owner
summary: The countries that may own the type.
see_also: ["system:production"]
when_omitted:
  kind: value
  value: ""
---

The value is a comma-separated list of country IDs, matched without regard to case; each recognized name adds its country and a name matching none adds nothing at all rather than reporting a problem.

```ini title="rules.ini"
[MYWEAP] ; example war factory BuildingType
Owner=GDI,Nod
```

Production reads the list twice. A BuildingType with no countries at all is never buildable, and one that names any additionally needs an owned construction yard [acting as](/keys/actslike/) one of them. For every kind of object, the factory that builds it must share at least one country with it, so a factory and a product that name disjoint lists never pair up. The computer's [base planning](/systems/ai-base-building/) and the role lists it is handed test the country the house acts as against the same list.

A country's place in the rules list is a bit in a 32-bit mask, so at most thirty-two countries can own anything; a country beyond the thirty-second owns nothing. [`DoubleOwned=yes`](/keys/doubleowned/) answers both questions with every country instead, outside campaign games.
