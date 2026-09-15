---
key: Description
scope: map-packets
label: Packet list entry
see_also: [CD, MinPlayers]
when_omitted:
  kind: value
  value: ""
  note: The map is listed with a blank description.
---

A scenario packet lists its maps in `[MultiMaps]`, and each entry names the section the map describes itself in. The text read from that section is the row the map occupies in the multiplayer scenario list. The host sends the same text to the other machines as the scenario name.

```ini title="MyMaps.PKT"
[MultiMaps]
1=MYMAP

[MYMAP] ; the section the entry above named
Description=Four player canyon
```

At most 43 characters are kept; a longer line is cut. The map file the entry stands for is the section name with `.MAP` appended. That file is read only for the digest the other players' copies are checked against, never for a description. A loose map file is read for its description instead.
