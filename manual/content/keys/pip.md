---
key: Pip
summary: The color of the pip a transport draws for this soldier while it is carried.
see_also: [Passengers, PipScale, Size, "system:transports"]
when_omitted:
  kind: value
  value: green
---

A transport draws one pip per passenger slot up to five, and only where [`PipScale`](/keys/pipscale/) selects the passenger scale, coloring each one from what is standing in it: a slot holding an infantry takes that infantry type's setting, a slot holding anything else is green, and an empty slot is drawn empty. Nothing else reads the value — a soldier on the ground draws no pip of its own, and the setting has no meaning on a type that never rides anywhere.

```ini title="rules.ini"
[MYCOMMANDO] ; example InfantryType
Pip=white
```

:::caution[An unrecognized name resets the color to `green`]
Omitting the key keeps whatever an earlier rules layer set, but a value that matches none of the [pip colors](/reference/enums/pip-color/) does not: it discards the stored value and stores `green` instead. A misspelling in a later layer therefore silently undoes an earlier assignment.
:::
