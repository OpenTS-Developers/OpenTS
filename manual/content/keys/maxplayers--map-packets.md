---
key: MaxPlayers
scope: map-packets
label: Packet-listed player ceiling
see_also: [MinPlayers, Description]
no_effect: true
when_omitted:
  kind: value
  value: "4"
---

```ini title="MyMaps.PKT"
[MYMAP] ; a section named by the packet's [MultiMaps] list
MinPlayers=2
MaxPlayers=4
```

The value is read into the entry's maximum player count. The count belongs to the listing and nothing reads it afterwards: not the lobby, not the starting-position pass, not the house assignment. A map declaring `MaxPlayers=4` can still be started with eight players.
