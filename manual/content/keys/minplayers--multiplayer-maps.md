---
key: MinPlayers
scope: multiplayer-maps
label: Loose map player limits
see_also: [MaxPlayers, Description, Official]
no_effect: true
when_omitted:
  kind: value
  value: "2"
---

```ini title="MyMap.MPR"
[Multiplay]
MinPlayers=2
```

A loose `.MPR` in the game directory declares its player limits in its own `[Multiplay]` section. The value is read into the listing's minimum player count. That count is private to the listing and nothing reads it afterwards, so the declared limit restricts nothing.

The section is read only while the map file is available; a listing for a file that has gone missing keeps the initial counts of `2` and `4`.
