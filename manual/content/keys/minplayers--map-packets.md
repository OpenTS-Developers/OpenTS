---
key: MinPlayers
scope: map-packets
label: Packet-listed player limits
see_also: [MaxPlayers, Description]
no_effect: true
when_omitted:
  kind: value
  value: "2"
---

```ini title="MyMaps.PKT"
[MYMAP] ; a section named by the packet's [MultiMaps] list
MinPlayers=2
```

The value is read into the entry's minimum player count. The count belongs to the listing and nothing reads it afterwards: not the lobby, not the starting-position pass, not the house assignment. A map declaring `MinPlayers=2` can still be started with one player.
