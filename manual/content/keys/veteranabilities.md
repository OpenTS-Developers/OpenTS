---
key: VeteranAbilities
summary: The abilities an object of this type gains at veteran rank and keeps at elite.
see_also: ["system:veterancy"]
when_omitted:
  kind: value
  value: ""
---

A comma-separated list of ability tokens, matched without regard to letter case. An object of this type gains everything named here once it reaches veteran, and keeps all of it after a further promotion to elite, where [`EliteAbilities`](/keys/eliteabilities/) is added on top. [The ability table](/systems/veterancy/#abilities) lists the eighteen accepted tokens and what each one does.

```ini title="rules.ini"
[MYTANK] ; example UnitType
VeteranAbilities=FIREPOWER,ROF,SIGHT
```

An unrecognized token is discarded without complaint. The list replaces rather than merges, so a later rules layer that sets this key starts from an empty set and keeps only the tokens it names. Omitting the key in that layer leaves the earlier list in force, and so does an empty value: an assignment with nothing after the `=` is discarded when the file is read. A list therefore cannot be cleared by leaving the key out or by emptying it.

:::caution[A space after a comma silences the token]
Whitespace is stripped from the value as a whole, not from each token, so `VeteranAbilities=FIREPOWER, ROF` grants only `FIREPOWER`. Write the separators without spaces.
:::
