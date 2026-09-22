---
key: Pip
summary: The color of the pip a transport draws for this soldier while it is carried.
see_also: [Passengers, PipScale, MaxPips, Size, "system:transports"]
when_omitted:
  kind: value
  value: green
---

A selected transport's pip row shows its hold, one pip per unit of passenger space. An infantry passenger's pips take its type's `Pip` color, any other passenger's pips are green, and free space is drawn empty. The row's length comes from the transport's [`PipScale`](/keys/pipscale/) and [`MaxPips`](/keys/maxpips/): under `PipScale=Passengers` it is five pips, or `MaxPips` where set, but never more than the transport's [`Passengers`](/keys/passengers/). Passengers past the end of the row get no pip. Nothing else reads the value, so it has no effect on an infantry type that never rides in a transport.

```ini title="rules.ini"
[MYCOMMANDO] ; example InfantryType
Pip=white
```

:::caution[An unrecognized name resets the color to `green`]
Omitting the key keeps whatever an earlier rules layer set, but a value that matches none of the [pip colors](/reference/enums/pip-color/) does not: it discards the stored value and stores `green` instead. A misspelling in a later layer therefore silently undoes an earlier assignment.
:::
