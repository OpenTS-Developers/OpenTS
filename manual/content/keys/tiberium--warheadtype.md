---
key: Tiberium
scope: warheadtype
label: Sets Tiberium off
see_also: ["system:tiberium", "ChainReaction"]
when_omitted:
  kind: value
  value: "no"
---

A [`ChainReaction=yes`](/keys/chainreaction/) Tiberium overlay detonates under a warhead with this flag; under every other warhead the overlay is inert. A sonic wave sets it off with no warhead check. The flag means nothing to overlays that are not Tiberium.
