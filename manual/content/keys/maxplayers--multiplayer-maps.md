---
key: MaxPlayers
scope: multiplayer-maps
label: Loose map player ceiling
see_also: [MinPlayers, Description, Official]
no_effect: true
when_omitted:
  kind: value
  value: "4"
---

```ini title="MyMap.MPR"
[Multiplay]
MinPlayers=2
MaxPlayers=4
```

A loose `.MPR` in the game directory declares its player limits in its own `[Multiplay]` section. The value is read into the listing's maximum player count, which is private to the listing: a map declaring `MaxPlayers=4` can still be started with eight players.

The section is read only while the map file is available; a listing for a file that has gone missing keeps the initial counts of `2` and `4`.
