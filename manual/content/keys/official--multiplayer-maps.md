---
key: Official
scope: multiplayer-maps
label: Multiplayer map provenance
see_also: ["Name"]
when_omitted:
  kind: value
  value: "no"
---

The `Official=` entry sits in the map's `[Basic]` section. Loose `.mpr` files in the game directory are scanned whenever the multiplayer map list is rebuilt. The flag travels with the map into the lobby: the host sends it to the guests along with the rest of the game options. It decides whether a guest who does not already hold the map may fetch it from the host.

A guest missing a map marked `yes` will not ask for it: on a LAN it reports that it cannot play and signs off. A map left unmarked is transferred from the host instead.

The generated random map is exempt from the refusal whatever the flag says. Maps packaged inside a `.pkt` list count as official without reading any setting.
